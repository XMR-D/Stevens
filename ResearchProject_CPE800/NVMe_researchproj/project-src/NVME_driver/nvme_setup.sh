#!/bin/sh

# NVMe Class Code is 010802
NVME_CLASS="0x010802"
RESOURCE_PATH=""
BDF=""

# 1. Scan /sys/bus/pci/devices to find the NVMe controller
for dev in /sys/bus/pci/devices/*; do
    # Check if the class matches our NVMe controller identifier
    if [ -f "$dev/class" ] && [ "$(cat "$dev/class")" = "$NVME_CLASS" ]; then
        BDF=$(basename "$dev")
        echo "NVMe controller found at: $BDF"
        RESOURCE_PATH="/sys/bus/pci/devices/$BDF/resource0"
        break
    fi
done

if [ -z "$BDF" ]; then
    echo "Error: No NVMe controller found."
    exit 1
fi

# 2. Unbind the native kernel driver to take manual control
# This is required so the kernel does not interfere with our direct MMIO access
if [ -d "/sys/bus/pci/devices/$BDF/driver" ]; then
    echo "Unbinding the device from its current kernel driver..."
    echo "$BDF" > "/sys/bus/pci/devices/$BDF/driver/unbind"
fi

echo "Device $BDF is isolated and ready for manual MMIO access."
echo "please call the driver with : ./nvme_driver $RESOURCE_PATH $BDF"