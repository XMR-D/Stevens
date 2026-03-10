#!/bin/sh

# NVMe Class Code is 010802
NVME_CLASS="0x010802"
RESOURCE_PATH=""
BDF=""

# 1. Scan /sys/bus/pci/devices to find the NVMe controller
for dev in /sys/bus/pci/devices/*; do
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

# 2. Unbind the native kernel driver (seulement si nécessaire)
if [ -d "/sys/bus/pci/devices/$BDF/driver" ]; then
    echo "Unbinding the device from its current kernel driver..."
    echo "$BDF" > "/sys/bus/pci/devices/$BDF/driver/unbind"
    sleep 0.5 
fi

# 3. ACTIVATE MEMORY ACCESS (Toujours exécuté)
# On utilise $BDF au lieu de l'adresse fixe pour éviter les erreurs
echo "Enabling Memory Space and Bus Master for $BDF..."
echo -ne "\x06\x00" | dd of="/sys/bus/pci/devices/$BDF/config" bs=1 seek=4 conv=notrunc

# 4. Vérification immédiate
VAL=$(hexdump -s 4 -n 2 -e '"%x"' "/sys/bus/pci/devices/$BDF/config")
if [ "$VAL" != "6" ]; then
    echo "Error : PCI Configuration failed (Value: $VAL)."
    exit 1
fi

echo "Device $BDF is isolated and ready for manual MMIO access."
echo "Please call the driver with : ./nvme_driver $RESOURCE_PATH $BDF"