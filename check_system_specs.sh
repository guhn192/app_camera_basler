#!/bin/bash

echo "=========================================="
echo "Basler Camera System Specifications Check"
echo "=========================================="
echo ""

echo "1. SYSTEM INFORMATION"
echo "====================="
echo "OS: $(lsb_release -d | cut -f2)"
echo "Kernel: $(uname -r)"
echo "Architecture: $(uname -m)"
echo ""

echo "2. HARDWARE SPECIFICATIONS"
echo "=========================="
echo "CPU: $(lscpu | grep 'Model name' | cut -d: -f2 | xargs)"
echo "CPU Cores: $(nproc)"
echo "Memory: $(free -h | grep Mem | awk '{print $2}')"
echo "Disk Space: $(df -h / | tail -1 | awk '{print $4}') available"
echo ""

echo "3. NETWORK INTERFACES"
echo "====================="
echo "Ethernet Interfaces:"
ip addr show | grep -E "^[0-9]+:" | grep -E "(enp|eth)" | while read line; do
    interface=$(echo $line | cut -d: -f2 | xargs)
    ip=$(ip addr show $interface | grep "inet " | awk '{print $2}')
    mtu=$(ip link show $interface | grep mtu | awk '{print $5}')
    echo "  $interface: $ip (MTU: $mtu)"
done
echo ""

echo "4. SOFTWARE VERSIONS"
echo "==================="
echo "GCC: $(gcc --version | head -1)"
echo "Qt: $(qmake --version | grep "Qt version" | cut -d' ' -f4)"
echo "OpenCV: $(pkg-config --modversion opencv4 2>/dev/null || echo "Not found")"
echo ""

echo "5. PYLON SDK"
echo "============"
if [ -d "/opt/pylon" ]; then
    echo "Pylon SDK: Installed at /opt/pylon"
    echo "PylonGigEConfigurator: $(ls -la /opt/pylon/bin/PylonGigEConfigurator 2>/dev/null | wc -l | xargs) (0=not found, 1=found)"
    echo "PylonViewer: $(ls -la /opt/pylon/bin/pylonviewer 2>/dev/null | wc -l | xargs) (0=not found, 1=found)"
else
    echo "Pylon SDK: Not installed"
fi
echo ""

echo "6. CAMERA DETECTION"
echo "=================="
if [ -f "/opt/pylon/bin/PylonGigEConfigurator" ]; then
    echo "Running PylonGigEConfigurator list..."
    sudo /opt/pylon/bin/PylonGigEConfigurator list 2>/dev/null | head -10
else
    echo "PylonGigEConfigurator not found"
fi
echo ""

echo "7. ENVIRONMENT VARIABLES"
echo "======================="
echo "LD_LIBRARY_PATH: ${LD_LIBRARY_PATH:-Not set}"
echo "PYLON_ROOT: ${PYLON_ROOT:-Not set}"
echo ""

echo "8. SYSTEM PERFORMANCE"
echo "===================="
echo "CPU Load: $(uptime | awk -F'load average:' '{print $2}')"
echo "Memory Usage: $(free | grep Mem | awk '{printf "%.1f%%", $3/$2 * 100.0}')"
echo ""

echo "=========================================="
echo "System check completed!"
echo "==========================================" 