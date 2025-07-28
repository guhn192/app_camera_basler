# Basler GigE 카메라 설정 및 앱 실행 가이드

## 목차
1. [시스템 요구사항](#시스템-요구사항)
2. [필수 패키지 설치](#필수-패키지-설치)
3. [Pylon SDK 설치](#pylon-sdk-설치)
4. [네트워크 설정 및 카메라 감지](#네트워크-설정-및-카메라-감지)
5. [앱 빌드 및 실행](#앱-빌드-및-실행)
6. [앱 사용법](#앱-사용법)
7. [문제 해결](#문제-해결)
8. [성능 최적화](#성능-최적화)
9. [주요 파일 및 디렉토리](#주요-파일-및-디렉토리)

---

## 시스템 요구사항

### 하드웨어 요구사항
- **CPU**: Intel Core i5 이상 또는 동등한 성능
- **메모리**: 최소 4GB RAM (8GB 이상 권장)
- **네트워크**: GigE 네트워크 어댑터 (Intel PRO 1000, I210, I340, I350 시리즈 권장)
- **디스크**: 최소 2GB 여유 공간 (SSD 권장)

### 소프트웨어 요구사항
- **OS**: Ubuntu 18.04 이상
- **컴파일러**: GCC 7.0 이상
- **Qt**: Qt 5.12 이상
- **OpenCV**: 4.x 버전

---

## 필수 패키지 설치

### 기본 개발 도구 설치
```bash
# 시스템 업데이트
sudo apt update && sudo apt upgrade

# 필수 패키지 설치
sudo apt-get install build-essential cmake git

# Qt5 및 개발 도구
sudo apt-get install qt5-default qtcreator

# OpenCV
sudo apt-get install libopencv-dev

# ethtool (네트워크 최적화용)
sudo apt-get install ethtool
```

---

## Pylon SDK 설치

### 1. Pylon SDK 다운로드
- Basler 웹사이트 방문: https://www.baslerweb.com/
- "Downloads" → "Software Downloads" → "pylon Software Suite"
- Linux용 Pylon SDK 다운로드

### 2. Pylon SDK 설치
```bash
# 다운로드한 파일 압축 해제
tar -xzf pylon-*.tar.gz

# 설치 디렉토리 생성
sudo mkdir -p /opt/pylon

# Pylon SDK 설치
sudo tar -C /opt/pylon -xzf pylon-*.tar.gz

# 권한 설정
sudo chmod 755 /opt/pylon
```

### 3. 환경 변수 설정
```bash
# 환경 변수 설정 스크립트 실행
source /opt/pylon/bin/pylon-setup-env.sh /opt/pylon

# 영구 설정을 위해 ~/.bashrc에 추가
echo "source /opt/pylon/bin/pylon-setup-env.sh /opt/pylon" >> ~/.bashrc
source ~/.bashrc
```

---

## 네트워크 설정 및 카메라 감지

### 1. 네트워크 어댑터 확인
```bash
# 네트워크 인터페이스 확인
ip addr show

# Pylon이 감지하는 네트워크 어댑터 확인
sudo /opt/pylon/bin/PylonGigEConfigurator list
```

**예상 출력:**
```
Found 4 network adapter(s).
Friendly name:  enp1s0
IP addresses:   IP: 192.168.0.1, Mask: 255.255.255.0 [24]
Friendly name:  enp2s0
IP addresses:   IP: 192.168.0.4, Mask: 255.255.255.0 [24]
```

### 2. 자동 네트워크 최적화
```bash
# 모든 설정 자동 최적화 (권장)
sudo /opt/pylon/bin/PylonGigEConfigurator auto-all

# 또는 개별적으로 실행
sudo /opt/pylon/bin/PylonGigEConfigurator auto-opt  # 네트워크 최적화만
sudo /opt/pylon/bin/PylonGigEConfigurator auto-ip   # IP 설정만
```

**성공적인 최적화 출력:**
```
Found device: Basler a2A1920-51gmPRO (00:30:53:35:E1:35)
Optimizing adapter enp2s0: Setting packet size to 9000.
Adjusted rtprio to 99 in /etc/security/limits.conf.
Disabled rp_filter for network enp2s0.
Adjusted net.core.rmem_max value to 32 MB.
```

### 3. 카메라 감지 확인
```bash
# 테스트 프로그램으로 카메라 감지 확인
LD_LIBRARY_PATH=/opt/pylon/lib ./test_pylon
```

**성공적인 감지 출력:**
```
Pylon initialized successfully
Total devices found: 1

Device 0:
  Friendly Name: Basler a2A1920-51gmPRO (40058160)
  Model Name: a2A1920-51gmPRO
  Serial Number: 40058160
  Device Class: BaslerGigE
  IP Address: 192.168.3.3
  MAC Address: 00305335E135
```

---

## 앱 빌드 및 실행

### 1. 프로젝트 디렉토리로 이동
```bash
cd app_camera_basler
```

### 2. 프로젝트 빌드
```bash
# 기존 빌드 파일 정리
rm -f Makefile

# 새로 빌드
qmake
make
```

### 3. 앱 실행
```bash
./app_camera_basler
```

---

## 앱 사용법

### 1. 카메라 연결
1. **IP 주소 설정**: "Camera IP Address" 섹션에서 카메라 IP 입력
   - 기본값: `192.168.3.3` (PylonGigEConfigurator가 설정한 IP)
2. **"Set IP"** 버튼 클릭
3. **"Connect"** 버튼 클릭하여 카메라 연결

### 2. 이미지 캡처
1. **"Start Grabbing"** 버튼 클릭하여 실시간 이미지 캡처 시작
2. **"Stop Grabbing"** 버튼 클릭하여 캡처 중지
3. **"Disconnect"** 버튼 클릭하여 카메라 연결 해제

### 3. 카메라 설정 조정
- **해상도**: Resolution Control 섹션에서 설정
- **노출 시간**: Exposure Control 섹션에서 조정
- **프레임 레이트**: Frame Rate Control 섹션에서 설정
- **트리거**: Trigger Control 섹션에서 설정

### 4. 이미지 녹화
- **녹화 활성화**: Recording Control 섹션에서 "Start Recording" 클릭
- **녹화 경로**: 녹화할 이미지 저장 경로 설정
- **최대 이미지 수**: 한 번에 저장할 최대 이미지 수 설정

---

## 문제 해결

### 1. 카메라가 감지되지 않는 경우

#### 1.1 네트워크 설정 확인
```bash
# 네트워크 인터페이스 상태 확인
ip addr show

# Pylon 설정 확인
sudo /opt/pylon/bin/PylonGigEConfigurator list
```

#### 1.2 자동 최적화 실행
```bash
# 네트워크 최적화
sudo /opt/pylon/bin/PylonGigEConfigurator auto-all
```

#### 1.3 카메라 감지 테스트
```bash
# 테스트 프로그램 실행
LD_LIBRARY_PATH=/opt/pylon/lib ./test_pylon
```

#### 1.4 IP Configurator 사용
```bash
# GUI 도구로 카메라 확인
sudo /opt/pylon/bin/ipconfigurator
```

### 2. 빌드 오류가 발생하는 경우

#### 2.1 의존성 확인
```bash
# 필수 패키지 재설치
sudo apt-get install qt5-default libopencv-dev build-essential

# Pylon SDK 경로 확인
ls -la /opt/pylon/
```

#### 2.2 환경 변수 설정
```bash
# 환경 변수 설정
export LD_LIBRARY_PATH=/opt/pylon/lib:$LD_LIBRARY_PATH
export PYLON_ROOT=/opt/pylon
```

### 3. 런타임 오류가 발생하는 경우

#### 3.1 권한 설정
```bash
# USB 카메라 권한 설정 (USB 카메라의 경우)
sudo /opt/pylon/share/pylon/setup-usb.sh

# 실시간 스레드 우선순위 설정
echo "* - rtprio 99" | sudo tee -a /etc/security/limits.conf
```

#### 3.2 시스템 재부팅
```bash
# 설정 변경 후 재부팅
sudo reboot
```

### 4. 일반적인 오류 메시지

#### 4.1 "No camera found"
- 네트워크 설정 확인
- PylonGigEConfigurator 실행
- 카메라 IP 주소 확인

#### 4.2 "Connection failed"
- 네트워크 케이블 연결 확인
- 방화벽 설정 확인
- 카메라 전원 확인

#### 4.3 "Permission denied"
- 사용자 권한 확인
- udev 규칙 설정
- sudo 권한으로 실행

---

## 성능 최적화

### 1. 네트워크 최적화
```bash
# Jumbo frames 활성화
sudo ip link set enp2s0 mtu 9000

# UDP 수신 버퍼 크기 증가
echo 'net.core.rmem_max=33554432' | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

### 2. 시스템 최적화
```bash
# 실시간 스레드 우선순위 설정
echo "* - rtprio 99" | sudo tee -a /etc/security/limits.conf

# 파일 핸들 제한 증가
echo "* hard nofile 4096" | sudo tee -a /etc/security/limits.conf
```

### 3. 카메라 설정 최적화
- **패킷 크기**: 8192로 설정
- **프레임 레이트**: 필요에 따라 조정
- **노출 시간**: 조명 조건에 맞게 조정

---

## 주요 파일 및 디렉토리

### 프로젝트 구조
```
app_camera_basler/
├── app_camera_basler.pro    # Qt 프로젝트 파일
├── main.cpp                 # 메인 애플리케이션
├── mainwindow.h/cpp         # GUI 인터페이스
├── basler_camera.h/cpp      # 카메라 제어 클래스
├── mainwindow.ui            # UI 디자인 파일
├── test_pylon.cpp           # 카메라 감지 테스트 프로그램
├── BASLER_CAMERA_SETUP_GUIDE.md  # 이 가이드 파일
└── README.md               # 프로젝트 문서
```

### Pylon 관련 파일
```
/opt/pylon/
├── bin/
│   ├── PylonGigEConfigurator  # 네트워크 최적화 도구
│   ├── ipconfigurator         # IP 설정 도구
│   └── pylon-setup-env.sh     # 환경 변수 설정 스크립트
├── lib/                       # 라이브러리 파일
│   ├── libpylonbase.so
│   ├── libpylonutility.so
│   └── libpylonc.so
└── share/pylon/              # 설정 파일 및 문서
    ├── setup-usb.sh          # USB 권한 설정
    └── README                # Pylon 문서
```

---

## 로그 확인

### 1. 앱 로그
앱 실행 시 콘솔에 출력되는 로그를 통해 다음을 확인할 수 있습니다:
- 카메라 연결 상태
- 이미지 캡처 상태
- 설정 변경 사항
- 오류 메시지

### 2. 시스템 로그
```bash
# 네트워크 관련 로그
dmesg | grep -i eth

# Pylon 관련 로그
journalctl | grep -i pylon

# 시스템 성능 모니터링
htop
```

---

## 카메라 사양

### Basler a2A1920-51gmPRO
- **해상도**: 1920 x 1200
- **프레임 레이트**: 최대 22 FPS
- **센서**: CMOS
- **인터페이스**: GigE Vision
- **IP 주소**: 192.168.3.3 (설정에 따라 변경 가능)
- **MAC 주소**: 00:30:53:35:E1:35

---

## 추가 리소스

### 공식 문서
- [Basler Pylon Documentation](https://docs.baslerweb.com/)
- [GigE Vision Standard](https://www.emva.org/standards-technology/gige-vision/)
- [Qt Documentation](https://doc.qt.io/)

### 유용한 도구
- **Pylon Viewer**: 카메라 테스트 및 설정
- **IP Configurator**: 네트워크 설정
- **PylonGigEConfigurator**: 자동 최적화

---

## 지원 및 문의

### 문제 해결 순서
1. 이 가이드의 [문제 해결](#문제-해결) 섹션 확인
2. 시스템 로그 확인
3. Basler 기술 지원 문의
4. 커뮤니티 포럼 검색

### 유용한 명령어 모음
```bash
# 네트워크 상태 확인
ip addr show
ping 192.168.3.3

# Pylon 도구
sudo /opt/pylon/bin/PylonGigEConfigurator list
sudo /opt/pylon/bin/ipconfigurator

# 앱 빌드 및 실행
qmake && make
./app_camera_basler

# 테스트
LD_LIBRARY_PATH=/opt/pylon/lib ./test_pylon
```

---

**마지막 업데이트**: 2024년 7월 28일  
**버전**: 1.0  
**작성자**: AI Assistant 