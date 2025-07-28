# Basler Camera Qt Application

Basler GigE 카메라를 사용하여 실시간 이미지를 캡처하고 표시하는 Qt 애플리케이션입니다.

## 🚀 빠른 시작

### 자동 설치 (권장)
```bash
# 설치 스크립트 실행
./install_basler_camera.sh
```

### 수동 설치
자세한 설치 가이드는 [BASLER_CAMERA_SETUP_GUIDE.md](BASLER_CAMERA_SETUP_GUIDE.md)를 참조하세요.

## 📋 요구사항

- Ubuntu 18.04 이상
- Qt 5.12 이상
- OpenCV 4.x
- Basler Pylon SDK
- GigE 네트워크 어댑터

## 기능

- Basler 카메라 자동 연결 및 감지
- 실시간 이미지 캡처 및 표시
- 다양한 픽셀 포맷 지원 (Mono8, RGB8, BGR8, Mono12, Mono16)
- 카메라 정보 표시 (이름, 모델, 시리얼 번호)
- 직관적인 GUI 인터페이스

## 요구사항

### 시스템 요구사항
- Linux (Ubuntu 18.04 이상 권장)
- Qt 5.12 이상
- OpenCV 4.x
- Basler Pylon SDK

### 설치된 라이브러리
- Qt5 (core, gui, widgets)
- OpenCV (core, highgui, imgcodecs, imgproc)
- Basler Pylon SDK

## 빌드 및 실행

### 1. 의존성 설치

```bash
# Qt5 설치 (Ubuntu)
sudo apt-get install qt5-default qtcreator

# OpenCV 설치
sudo apt-get install libopencv-dev

# Basler Pylon SDK 설치
# Basler 웹사이트에서 다운로드하여 설치
# https://www.baslerweb.com/en/sales-support/downloads/software-downloads/pylon-6-3-0-linux/
```

### 2. 프로젝트 빌드

```bash
# 프로젝트 디렉토리로 이동
cd app_camera_basler

# qmake 실행
qmake

# 빌드
make

# 또는 한 번에 실행
make clean && qmake && make
```

### 3. 애플리케이션 실행

```bash
./app_camera_basler
```

## 사용법

1. **애플리케이션 시작**: `./app_camera_basler` 명령으로 애플리케이션을 실행합니다.

2. **카메라 연결**: "Connect" 버튼을 클릭하여 Basler 카메라에 연결합니다.

3. **이미지 캡처 시작**: "Start Grabbing" 버튼을 클릭하여 실시간 이미지 캡처를 시작합니다.

4. **이미지 캡처 중지**: "Stop Grabbing" 버튼을 클릭하여 이미지 캡처를 중지합니다.

5. **카메라 연결 해제**: "Disconnect" 버튼을 클릭하여 카메라 연결을 해제합니다.

## 프로젝트 구조

```
app_camera_basler/
├── app_camera_basler.pro    # Qt 프로젝트 파일
├── main.cpp                 # 메인 애플리케이션 진입점
├── mainwindow.h             # 메인 윈도우 헤더
├── mainwindow.cpp           # 메인 윈도우 구현
├── mainwindow.ui            # Qt Designer UI 파일
├── basler_camera.h          # Basler 카메라 클래스 헤더
├── basler_camera.cpp        # Basler 카메라 클래스 구현
└── README.md               # 이 파일
```

## 주요 클래스

### BaslerCamera
- Basler 카메라와의 통신을 담당
- 실시간 이미지 캡처 및 처리
- 다양한 픽셀 포맷 지원
- 스레드 기반 이미지 캡처

### MainWindow
- Qt GUI 인터페이스
- 카메라 제어 버튼
- 실시간 이미지 표시
- 카메라 정보 표시

## 지원되는 카메라 포맷

- **Mono8**: 8비트 그레이스케일
- **RGB8packed**: 8비트 RGB
- **BGR8packed**: 8비트 BGR
- **Mono12**: 12비트 그레이스케일
- **Mono16**: 16비트 그레이스케일

## 문제 해결

### 카메라가 감지되지 않는 경우
1. Basler Pylon SDK가 올바르게 설치되었는지 확인
2. 카메라가 USB에 연결되어 있는지 확인
3. 카메라 드라이버가 설치되어 있는지 확인

### 빌드 오류가 발생하는 경우
1. 모든 의존성 라이브러리가 설치되어 있는지 확인
2. Basler Pylon SDK 경로가 올바른지 확인
3. OpenCV 라이브러리 경로가 올바른지 확인

### 런타임 오류가 발생하는 경우
1. 카메라가 다른 애플리케이션에서 사용 중인지 확인
2. 카메라 권한이 올바른지 확인
3. 시스템 로그를 확인하여 오류 메시지 확인

## 라이선스

이 프로젝트는 MIT 라이선스 하에 배포됩니다.

## 기여

버그 리포트나 기능 요청은 GitHub Issues를 통해 제출해주세요.

## 연락처

문의사항이 있으시면 프로젝트 관리자에게 연락해주세요. 