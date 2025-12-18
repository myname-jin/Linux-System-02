# 💬 OpenTalk: 리눅스 기반 GUI 채팅 & 파일 전송 메신저

> **동의대학교 컴퓨터소프트웨어공학과 3학년 2학기 오픈소스 프로젝트**
> **TCP/IP 소켓 통신과 GTK+ 라이브러리를 활용한 멀티스레드 메신저 구현**

![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat-square&logo=linux&logoColor=black)
![C](https://img.shields.io/badge/C-A8B9CC?style=flat-square&logo=c&logoColor=black)
![GTK+](https://img.shields.io/badge/GTK+_3.0-4B8BBE?style=flat-square&logo=gnome&logoColor=white)
![Socket](https://img.shields.io/badge/Socket_Programming-000000?style=flat-square&logo=socket.io&logoColor=white)

## 📌 프로젝트 소개
**OpenTalk**은 리눅스 환경에서 작동하는 **GUI 기반의 실시간 메신저 프로그램**입니다.
기존의 CLI(Command Line Interface) 채팅 프로그램의 한계를 넘어, **GTK+ 3.0 라이브러리**를 적용하여 사용자 친화적인 그래픽 인터페이스를 제공합니다.

**멀티스레드(Pthread)** 기술을 적용하여 다수의 클라이언트가 동시에 접속해도 끊김 없는 메시지 송수신이 가능하며, **파일 전송 프로토콜**을 직접 구현하여 텍스트뿐만 아니라 바이너리 데이터(이미지, 문서 등)까지 안정적으로 주고받을 수 있도록 설계하였습니다.

## 👥 팀원 구성 (4조)

| 학번 | 이름 | 역할 | 담당 업무 (Role) |
| :--- | :--- | :--- | :--- |
| **20212977** | **이규찬** | **팀장 (Server/Main)** | • 멀티스레드 서버 아키텍처 설계 (`pthread`)<br>• 클라이언트 접속/종료 및 세션 관리<br>• 메시지 브로드캐스팅 로직 구현<br>• 프로젝트 통합 및 일정 관리 |
| **20233065** | **이지민** | **팀원 (Client/UI)** | • GTK+ 3.0 기반 클라이언트 GUI 디자인 및 구현<br>• 로그인, 채팅방, 파일 전송 창 화면 구성<br>• 사용자 이벤트(버튼 클릭, 키 입력) 처리 |
| **20212979** | **임진호** | **팀원 (Network/File)** | • 파일 전송 프로토콜 설계 및 구현<br>• 소켓 통신 모듈화 (`send`/`recv` 패킷 처리)<br>• 대용량 데이터 버퍼링 및 예외 처리 |

## ✨ 주요 기능 (Features)

### 1. 실시간 멀티 채팅 (Multi-User Chatting)
- **TCP/IP 소켓 통신:** 안정적인 연결 지향형 통신으로 데이터 유실 없는 대화 가능.
- **멀티스레드 서버:** `Mutex`를 활용한 임계 구역 동기화로 여러 명이 동시에 말해도 꼬이지 않음.
- **브로드캐스트:** 한 명이 말하면 접속한 모든 사용자에게 즉시 메시지 전송.

### 2. 사용자 친화적 GUI (Graphical User Interface)
- **GTK+ 3.0:** 리눅스 표준 GUI 툴킷을 사용하여 깔끔하고 직관적인 화면 구성.
- **로그인 & 닉네임:** 접속 시 서버 IP와 포트, 사용할 닉네임을 입력받아 접속.
- **자동 스크롤:** 새로운 메시지가 도착하면 채팅창이 자동으로 아래로 스크롤됨.

### 3. 파일 전송 기능 (File Transfer)
- **바이너리 전송:** 텍스트뿐만 아니라 이미지, 문서 등 모든 형식의 파일 전송 지원.
- **전용 프로토콜:** `[FILE]:파일명:크기` 형태의 헤더를 정의하여 채팅 메시지와 파일 데이터를 구분 처리.

## 🛠️ 설치 및 실행 방법 (Getting Started)

### 사전 요구 사항
- **OS:** Linux (Ubuntu 22.04 권장)
- **Compiler:** GCC
- **Library:** GTK+ 3.0 개발 도구 (`libgtk-3-dev`)

### 라이브러리 설치 (Ubuntu 기준)
```bash
sudo apt-get update
sudo apt-get install libgtk-3-dev build-essential
```

### 빌드 및 실행
```bash
1. 컴파일 (Makefile 사용)
make

2. 서버 실행 (포트 번호 지정)
./server 8080

3. 클라이언트 실행 (새 터미널)
./client

4. 접속
- 사용자 이름(닉네임) 입력
- IP주소, 포트 입력 (자동 입력)
```

## 📂 디렉토리 구조 (Architecture)

```plaintext
OpenSource_Project2/
├─ server.c        # [Server] 다중 클라이언트 접속 처리 및 메시지 중계
├─ client.c        # [Client] GUI 실행 및 서버 통신 요청
├─ ui.c            # [Module] GTK+ 화면 구성 및 이벤트 콜백 함수
├─ ui.h            # [Header] UI 관련 함수 헤더
├─ network.c       # [Module] 소켓 생성, 패킷 송수신, 파일 전송 로직
├─ network.h       # [Header] 네트워크 관련 함수 헤더
├─ common.h        # [Common] 공통 상수, 구조체(패킷) 정의
├─ Makefile        # [Build] 빌드 자동화 스크립트
├─ LICENSE         # MIT License 명시
└─ README.md       # 프로젝트 문서
```

## 🛠 개발 환경

- **OS:** Ubuntu Linux
- **Language:** C (Standard C99)
- **Library:** GTK+ 3.0, Pthread
- **Tools:** GCC, Make, VSCode, Git
---
© 2025 Team 4. All Rights Reserved.
