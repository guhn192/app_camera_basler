#!/bin/bash

# Basler GigE Camera Setup Script
# 이 스크립트는 Basler GigE 카메라 설정을 자동화합니다.

set -e  # 오류 발생 시 스크립트 중단

echo "=========================================="
echo "Basler GigE Camera Setup Script"
echo "=========================================="

# 색상 정의
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 로그 함수
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 1. 시스템 업데이트
log_info "시스템 패키지 업데이트 중..."
sudo apt update && sudo apt upgrade -y

# 2. 필수 패키지 설치
log_info "필수 패키지 설치 중..."
sudo apt-get install -y build-essential cmake git qt5-default qtcreator libopencv-dev ethtool

# 3. Pylon SDK 확인
log_info "Pylon SDK 설치 확인 중..."
if [ ! -d "/opt/pylon" ]; then
    log_error "Pylon SDK가 설치되지 않았습니다."
    log_info "다음 단계를 따라 Pylon SDK를 설치하세요:"
    echo "1. https://www.baslerweb.com/ 방문"
    echo "2. Downloads → Software Downloads → pylon Software Suite"
    echo "3. Linux용 Pylon SDK 다운로드"
    echo "4. 다음 명령어로 설치:"
    echo "   sudo mkdir -p /opt/pylon"
    echo "   sudo tar -C /opt/pylon -xzf pylon-*.tar.gz"
    echo "   sudo chmod 755 /opt/pylon"
    exit 1
else
    log_success "Pylon SDK가 설치되어 있습니다."
fi

# 4. 환경 변수 설정
log_info "환경 변수 설정 중..."
if ! grep -q "pylon-setup-env.sh" ~/.bashrc; then
    echo "source /opt/pylon/bin/pylon-setup-env.sh /opt/pylon" >> ~/.bashrc
    log_success "환경 변수가 ~/.bashrc에 추가되었습니다."
else
    log_info "환경 변수가 이미 설정되어 있습니다."
fi

# 현재 세션에 환경 변수 적용
export LD_LIBRARY_PATH=/opt/pylon/lib:$LD_LIBRARY_PATH
export PYLON_ROOT=/opt/pylon

# 5. 네트워크 어댑터 확인
log_info "네트워크 어댑터 확인 중..."
echo "현재 네트워크 인터페이스:"
ip addr show | grep -E "inet.*192\.168"

# 6. Pylon 설정 확인
log_info "Pylon 네트워크 설정 확인 중..."
if [ -f "/opt/pylon/bin/PylonGigEConfigurator" ]; then
    sudo /opt/pylon/bin/PylonGigEConfigurator list
else
    log_error "PylonGigEConfigurator를 찾을 수 없습니다."
    exit 1
fi

# 7. 자동 최적화 실행 여부 확인
echo ""
read -p "네트워크 자동 최적화를 실행하시겠습니까? (y/n): " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    log_info "네트워크 자동 최적화 실행 중..."
    sudo /opt/pylon/bin/PylonGigEConfigurator auto-all
    log_success "네트워크 최적화가 완료되었습니다."
fi

# 8. 카메라 감지 테스트
log_info "카메라 감지 테스트 중..."
if [ -f "test_pylon" ]; then
    ./test_pylon
else
    log_warning "test_pylon 실행 파일이 없습니다. 먼저 빌드하세요."
fi

# 9. 앱 빌드
log_info "앱 빌드 중..."
if [ -f "app_camera_basler.pro" ]; then
    rm -f Makefile
    qmake
    make
    log_success "앱 빌드가 완료되었습니다."
else
    log_error "app_camera_basler.pro 파일을 찾을 수 없습니다."
    exit 1
fi

# 10. 권한 설정
log_info "권한 설정 중..."
if [ -f "/opt/pylon/share/pylon/setup-usb.sh" ]; then
    sudo /opt/pylon/share/pylon/setup-usb.sh
fi

# 실시간 스레드 우선순위 설정
if ! grep -q "rtprio" /etc/security/limits.conf; then
    echo "* - rtprio 99" | sudo tee -a /etc/security/limits.conf
    log_success "실시간 스레드 우선순위가 설정되었습니다."
fi

# 11. 최종 확인
echo ""
log_success "설치가 완료되었습니다!"
echo ""
echo "다음 단계:"
echo "1. 시스템 재부팅 (권장): sudo reboot"
echo "2. 앱 실행: ./app_camera_basler"
echo "3. 카메라 IP 주소를 192.168.3.3으로 설정"
echo "4. Connect 버튼 클릭"
echo ""
echo "문제가 발생하면 BASLER_CAMERA_SETUP_GUIDE.md 파일을 참조하세요."

# 12. 현재 카메라 상태 확인
echo ""
log_info "현재 카메라 상태:"
if [ -f "test_pylon" ]; then
    ./test_pylon
fi 