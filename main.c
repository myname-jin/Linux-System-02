/**
 * @file main.c
 * @brief 프로그램 진입점
 */

#include "common.h"
#include "ui.h"
#include "network.h"

int main(int argc, char *argv[]) {
    
    // GUI 초기화 및 실행
    gtk_init(&argc, &argv);
    activate_ui(argc, argv);

    // 종료 시 소켓 정리
    if (sock > 0) close(sock);
    return 0;
}