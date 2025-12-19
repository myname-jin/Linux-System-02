#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"

// 전역 변수 (다른 파일에서도 접근 가능)
extern int sock;
extern char my_name[MAX_NAME_LEN];

// 함수 원형
int connect_to_server(const char *ip, int port);
void send_text_message(const char *msg);
void* send_file_thread(void *arg);
void* recv_msg_thread(void *arg);
char* convert_to_utf8(const char *input);

#endif