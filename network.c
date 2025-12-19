/**
 * @file network.c
 * @brief 클라이언트의 네트워크 통신 로직 구현
 * @details 소켓 연결, 메시지 송수신, 파일 업로드/다운로드 로직을 포함합니다.
 */

#include "network.h"
#include "ui.h" 

// 전역 변수
int sock = -1;
char my_name[MAX_NAME_LEN];

// 파일 수신 상태 관리를 위한 정적 변수들
static int is_receiving_file = 0;
static FILE *recv_fp = NULL;
static long recv_remain_size = 0;
static char recv_filename[256];
static char temp_filepath[300];

/**
 * @brief CP949 인코딩 문자열을 UTF-8로 변환합니다.
 * @param input 변환할 원본 문자열
 * @return 변환된 UTF-8 문자열 (반드시 g_free로 해제 필요)
 */
char* convert_to_utf8(const char *input) {
    if (!input) return NULL;
    if (g_utf8_validate(input, -1, NULL)) return g_strdup(input);
    
    GError *error = NULL;
    char *utf8 = g_convert(input, -1, "UTF-8", "CP949", NULL, NULL, &error);
    if (error) {
        g_error_free(error);
        return g_strdup(input);
    }
    return utf8;
}

/**
 * @brief 서버에 TCP 연결을 시도합니다.
 * @param ip 서버 IP 주소
 * @param port 서버 포트 번호
 * @return 성공 시 0, 실패 시 -1
 */
int connect_to_server(const char *ip, int port) {
    struct sockaddr_in serv_addr;
    
    // 소켓 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket Creation Failed");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    // 연결 시도
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("Connection Failed");
        return -1;
    }
    
    // 접속 성공 시 닉네임 전송
    if (send(sock, my_name, strlen(my_name), 0) < 0) {
        perror("Send Name Failed");
        return -1;
    }
    
    LOG("Connected to Server %s:%d", ip, port);
    return 0;
}

/**
 * @brief 텍스트 메시지를 서버로 전송합니다.
 * @param msg 보낼 메시지 내용
 */
void send_text_message(const char *msg) {
    if (sock < 0) return;
    if (send(sock, msg, strlen(msg), 0) < 0) {
        perror("Message Send Failed");
    }
}

/**
 * @brief 파일을 서버로 전송하는 스레드 함수
 * @details 대용량 파일 전송 시 UI 블로킹을 막기 위해 별도 스레드에서 실행됩니다.
 */
void* send_file_thread(void *arg) {
    char *filename = (char *)arg;
    
    // 파일 열기
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        ERR("Cannot open file: %s", filename);
        g_free(filename);
        return NULL;
    }

    // 파일 크기 측정
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    rewind(fp);

    LOG("Start File Upload: %s (%ld bytes)", filename, filesize);

    // 1. 헤더 전송 [FILE]:파일명:크기
    char header[512];
    sprintf(header, "[FILE]:%s:%ld", g_path_get_basename(filename), filesize);
    send(sock, header, strlen(header), 0);
    
    // 서버 버퍼링 대기
    usleep(50000); 

    // 2. 데이터 청크 전송
    char buffer[BUFFER_SIZE];
    char send_buf[BUFFER_SIZE + 10];
    size_t read_size;
    long total_sent = 0;

    while ((read_size = fread(buffer, 1, BUFFER_SIZE - 6, fp)) > 0) {
        memcpy(send_buf, "[DATA]", 6);
        memcpy(send_buf + 6, buffer, read_size);
        
        if (send(sock, send_buf, read_size + 6, 0) < 0) {
            perror("File Data Send Error");
            break;
        }
        total_sent += read_size;
        usleep(1000); // 네트워크 혼잡 방지
    }
    
    LOG("File Upload Completed: %ld / %ld bytes", total_sent, filesize);

    fclose(fp);
    g_free(filename);
    
    add_system_msg("📤 파일 전송이 완료되었습니다.");
    return NULL;
}

/**
 * @brief 서버로부터 메시지를 수신하는 메인 스레드
 * @details 텍스트, 파일 헤더, 파일 데이터를 구분하여 처리합니다.
 */
void* recv_msg_thread(void *arg) {
    char buf[BUFFER_SIZE];
    int len;

    while ((len = recv(sock, buf, BUFFER_SIZE, 0)) > 0) {
        
        // ==========================================
        // CASE 1: 파일 데이터 수신 중
        // ==========================================
        if (is_receiving_file) {
            if (strncmp(buf, "[DATA]", 6) == 0) {
                int data_len = len - 6;
                // 남은 크기보다 더 많이 들어오면 자름 (안전장치)
                if (data_len > recv_remain_size) data_len = recv_remain_size;
                
                fwrite(buf + 6, 1, data_len, recv_fp);
                recv_remain_size -= data_len;

                // 다운로드 완료 체크
                if (recv_remain_size <= 0) {
                    fclose(recv_fp);
                    recv_fp = NULL;
                    is_receiving_file = 0;
                    
                    LOG("File Download Finished: %s", recv_filename);
                    
                    // UI에 다운로드 버튼 생성 요청
                    add_file_download_btn(recv_filename);
                }
            }
            continue;
        }

        // ==========================================
        // CASE 2: 새로운 파일 전송 시작 헤더 감지
        // ==========================================
        if (strncmp(buf, "[FILE]:", 7) == 0) {
            char *ptr = buf + 7;
            char *size_ptr = strchr(ptr, ':');
            
            if (size_ptr) {
                *size_ptr = '\0';
                strcpy(recv_filename, ptr);
                recv_remain_size = atol(size_ptr + 1);
                
                // 임시 파일 생성
                sprintf(temp_filepath, "temp_%s", recv_filename);
                recv_fp = fopen(temp_filepath, "wb");
                
                if (recv_fp) {
                    is_receiving_file = 1;
                    char alert[256];
                    sprintf(alert, "📂 파일 수신 중... (%s)", recv_filename);
                    add_system_msg(alert);
                } else {
                    ERR("Failed to create temp file");
                }
            }
            continue;
        }

        // ==========================================
        // CASE 3: 일반 텍스트 채팅
        // ==========================================
        buf[len] = 0;
        char *utf8 = convert_to_utf8(buf);
        char *sep = strchr(utf8, ':');
        
        if (sep) {
            *sep = '\0'; // 이름과 메시지 분리
            // 상대방 메시지로 UI에 추가
            add_chat_bubble(utf8, sep + 1, 0);
        } else {
            // 시스템 메시지 처리
            if (strstr(utf8, "[SYSTEM]")) {
                char *sys_sep = strchr(utf8, ':');
                if (sys_sep) add_system_msg(sys_sep + 1);
            }
        }
        g_free(utf8);
    }
    
    // 연결 종료 시
    LOG("Disconnected from server");
    add_system_msg("❌ 서버와의 연결이 끊어졌습니다.");
    return NULL;
}