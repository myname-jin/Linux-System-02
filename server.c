/**
 * @file server.c
 * @brief 다중 클라이언트 채팅 서버 (멀티스레드 기반)
 * @details 클라이언트의 접속, 메시지 중계, 파일 전송 패킷 라우팅을 담당합니다.
 */

#include "common.h"

// 클라이언트 관리 배열 (단순 int 배열 -> 구조체 배열로 업그레이드)
ClientInfo clients[MAX_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief 현재 접속자 목록을 서버 콘솔에 출력합니다.
 * 디버깅 및 관리자 모니터링 용도입니다.
 */
void print_client_list() {
    printf("========== Current Users ==========\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0) {
            printf("[%d] Name: %s (IP: %s)\n", 
                   i, 
                   clients[i].name, 
                   inet_ntoa(clients[i].address.sin_addr));
        }
    }
    printf("===================================\n");
}

/**
 * @brief 특정 클라이언트를 제외한 모든 인원에게 메시지를 전송합니다 (Broadcasting)
 * * @param data 전송할 데이터 버퍼
 * @param len 데이터의 길이
 * @param sender_sock 보낸 사람의 소켓 (Echo 방지)
 */
void broadcast(char *data, int len, int sender_sock) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0 && clients[i].socket != sender_sock) {
            // send 함수의 반환값을 확인하여 안정성 확보
            if (send(clients[i].socket, data, len, 0) < 0) {
                ERR("Failed to send message to client %d", clients[i].socket);
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

/**
 * @brief 개별 클라이언트 요청을 처리하는 스레드 함수
 * @param arg ClientInfo 구조체 포인터 또는 인덱스
 */
void *handle_client(void *arg) {
    int id = *(int *)arg;
    int sock = clients[id].socket;
    char buffer[BUFFER_SIZE];
    char name[MAX_NAME_LEN];
    int len;

    // 1. 닉네임 수신
    len = recv(sock, name, MAX_NAME_LEN, 0);
    if (len <= 0) {
        close(sock);
        clients[id].socket = 0; // 연결 해제 처리
        return NULL;
    }
    name[len] = 0;
    
    // 구조체에 이름 저장
    strcpy(clients[id].name, name);
    clients[id].join_time = time(NULL);

    LOG("User Connected: %s (Slot: %d)", name, id);
    print_client_list();

    // 2. 입장 알림 브로드캐스트
    sprintf(buffer, "[SYSTEM]:%s 님이 입장했습니다.", name);
    broadcast(buffer, strlen(buffer), sock);

    // 3. 메시지 루프
    while ((len = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        // [파일 전송] 헤더나 데이터는 그대로 통과 (Raw Forwarding)
        if (strncmp(buffer, "[FILE]:", 7) == 0 || strncmp(buffer, "[DATA]", 6) == 0) {
            broadcast(buffer, len, sock);
        } 
        // [일반 채팅] 이름 붙여서 전송
        else {
            buffer[len] = 0;
            char msg_full[BUFFER_SIZE + 50];
            sprintf(msg_full, "%s:%s", name, buffer); 
            broadcast(msg_full, strlen(msg_full), sock);
        }
    }

    // 4. 퇴장 처리
    LOG("User Disconnected: %s", clients[id].name);
    
    pthread_mutex_lock(&mutex);
    close(clients[id].socket);
    clients[id].socket = 0;
    memset(clients[id].name, 0, MAX_NAME_LEN);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

/**
 * @brief 서버 메인 함수
 * 소켓을 생성하고 클라이언트의 연결을 기다립니다.
 */
int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
    
    int serv_sock;
    struct sockaddr_in serv_addr;
    
    // 소켓 생성
    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        perror("Socket Error");
        return 1;
    }
    
    // 구조체 초기화
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    // Time-wait 상태의 포트 재사용 설정
    int opt = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 바인딩
    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("Bind Error");
        return 1;
    }
    
    // 리슨 시작
    if(listen(serv_sock, 5) == -1) {
        perror("Listen Error");
        return 1;
    }
    
    printf(">> Server Started on Port %s\n", argv[1]);
    printf(">> Waiting for connections...\n");

    // 클라이언트 수락 루프
    while(1) {
        struct sockaddr_in clnt_addr;
        socklen_t sz = sizeof(clnt_addr);
        int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &sz);
        
        pthread_mutex_lock(&mutex);
        for(int i=0; i<MAX_CLIENTS; i++) {
            if(clients[i].socket == 0) {
                clients[i].socket = clnt_sock;
                clients[i].address = clnt_addr;
                
                // 각 클라이언트 처리를 위한 스레드 생성
                pthread_t t;
                int *id = malloc(sizeof(int));
                *id = i;
                pthread_create(&t, NULL, handle_client, id);
                break;
            }
        }
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}