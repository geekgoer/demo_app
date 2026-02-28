#!/bin/bash

echo "=== RPMsg Echo Test ==="

# 1. 检查 rpmsg 设备
echo "1. 检查 rpmsg 设备："
ls -la /dev/rpmsg*
echo ""

# 2. 检查 rpmsg_ctrl 设备
echo "2. 检查 rpmsg_ctrl 设备："
ls -la /sys/class/rpmsg/
echo ""

# 3. 查看可用的 rpmsg 端点
echo "3. 查看 rpmsg 端点："
cat /sys/class/rpmsg/*/name 2>/dev/null
cat /sys/class/rpmsg/*/src 2>/dev/null
cat /sys/class/rpmsg/*/dst 2>/dev/null
echo ""

# 4. 测试 echo（如果设备存在）
if [ -e /dev/rpmsg0 ]; then
    echo "4. 发送测试消息到 R5："
    echo "Hello from A53!" > /dev/rpmsg0
    echo "消息已发送"
    echo ""
    
    echo "5. 读取 R5 的回复："
    timeout 2 cat /dev/rpmsg0
else
    echo "4. /dev/rpmsg0 不存在，检查 rpmsg_char 驱动是否加载"
    lsmod | grep rpmsg
fi

echo ""
echo "=== 测试完成 ==="
