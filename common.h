/**
 * @file common.h
 * @brief 프로젝트 전반에서 사용되는 상수, 매크로 및 데이터 구조체 정의
 * @date 2025-12-14
 * @author Team Project
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <time.h> // 시간 기록용

// ==========================================
//  프로젝트 설정 및 상수 정의
// ==========================================

/** 서버 포트 번호 기본값 */
#define DEFAULT_PORT 8080

/** 네트워크 버퍼 크기 (4KB) */
#define BUFFER_SIZE 4096

/** 사용자 이름 최대 길이 */
#define MAX_NAME_LEN 32

/** 최대 접속 가능 클라이언트 수 */
#define MAX_CLIENTS 20

// 프로토콜 헤더 정의 (통신 약속)
#define PROTOCOL_FILE "[FILE]"    ///< 파일 전송 헤더
#define PROTOCOL_DATA "[DATA]"    ///< 파일 데이터 청크 헤더
#define PROTOCOL_SYSTEM "[SYSTEM]"///< 시스템 알림 헤더

// ==========================================
//  데이터 구조체 정의
// ==========================================

/**
 * @brief UI와 로직 간 메시지 전달을 위한 구조체
 * 채팅 화면에 말풍선을 그리기 위해 필요한 모든 정보를 담습니다.
 */
typedef struct {
    char *name;          ///< 보낸 사람의 닉네임
    char *msg;           ///< 메시지 내용 (텍스트 또는 파일명)
    char timestamp[20];  ///< 메시지 전송 시간 (HH:MM:SS)
    int is_mine;         ///< 내가 보낸 메시지인지 여부 (1: true, 0: false)
    int is_file_btn;     ///< 파일 다운로드 버튼인지 여부 (1: true, 0: false)
    char *real_filename; ///< 실제 저장될 파일 이름 (파일 전송 시 사용)
} ChatData;

/**
 * @brief 서버에서 연결된 클라이언트 정보를 관리하는 구조체
 */
typedef struct {
    int socket;                 ///< 클라이언트 소켓 디스크립터
    struct sockaddr_in address; ///< 클라이언트 IP 주소 정보
    char name[MAX_NAME_LEN];    ///< 접속한 사용자 닉네임
    time_t join_time;           ///< 접속 시간
} ClientInfo;

// 디버깅용 매크로 (콘솔에 로그 출력)
#define LOG(fmt, ...) printf("[LOG] " fmt "\n", ##__VA_ARGS__)
#define ERR(fmt, ...) fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)

#endif // COMMON_H