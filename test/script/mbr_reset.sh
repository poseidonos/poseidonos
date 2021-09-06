#!/usr/bin/env bash

rootdir=$(readlink -f $(dirname $0))/../..
spdkdir=$rootdir/lib/spdk-20.10/scripts/

function pci_can_use() {
	local i

	# The '\ ' part is important
	if [[ " $PCI_BLACKLIST " =~ \ $1\  ]] ; then
		return 1
	fi

	if [[ -z "$PCI_WHITELIST" ]]; then
		#no whitelist specified, bind all devices
		return 0
	fi

	for i in $PCI_WHITELIST; do
		if [ "$i" == "$1" ] ; then
			return 0
		fi
	done

	return 1
}

function iter_all_pci_class_code() {
	local class="$(printf %02x $((0x$1)))"
	local subclass="$(printf %02x $((0x$2)))"
	local progif="$(printf %02x $((0x$3)))"

	if hash lspci &>/dev/null; then
		if [ "$progif" != "00" ]; then
			lspci -mm -n -D | \
				grep -i -- "-p${progif}" | \
				awk -v cc="\"${class}${subclass}\"" -F " " \
				'{if (cc ~ $2) print $1}' | tr -d '"'
		else
			lspci -mm -n -D | \
				awk -v cc="\"${class}${subclass}\"" -F " " \
				'{if (cc ~ $2) print $1}' | tr -d '"'
		fi
	elif hash pciconf &>/dev/null; then
		local addr=($(pciconf -l | grep -i "class=0x${class}${subclass}${progif}" | \
			cut -d$'\t' -f1 | sed -e 's/^[a-zA-Z0-9_]*@pci//g' | tr ':' ' '))
		printf "%04x:%02x:%02x:%x\n" ${addr[0]} ${addr[1]} ${addr[2]} ${addr[3]}
	else
		echo "Missing PCI enumeration utility"
		exit 1
	fi
}

function get_nvme_name_from_bdf {
	local blknames=()

	set +e
	nvme_devs=$(lsblk -d --output NAME | grep "^nvme")
	set -e
	for dev in $nvme_devs; do
		link_name=$(readlink /sys/block/$dev/device/device) || true
		if [ -z "$link_name" ]; then
			link_name=$(readlink /sys/block/$dev/device)
		fi
		link_bdf=$(basename "$link_name")
		if [ "$link_bdf" = "$1" ]; then
			blknames+=($dev)
		fi
	done

	printf '%s\n' "${blknames[@]}"
}

driver_path=""
driver_name=""
if [[ -n "${DRIVER_OVERRIDE}" ]]; then
    driver_path="${DRIVER_OVERRIDE%/*}"
    driver_name="${DRIVER_OVERRIDE##*/}"
    # path = name -> there is no path
    if [[ "$driver_path" = "$driver_name" ]]; then
        driver_path=""
    fi
elif [[ -n "$(ls /sys/kernel/iommu_groups)" || \
        (-e /sys/module/vfio/parameters/enable_unsafe_noiommu_mode && \
        "$(cat /sys/module/vfio/parameters/enable_unsafe_noiommu_mode)" == "Y") ]]; then
    driver_name=vfio-pci
elif modinfo uio_pci_generic >/dev/null 2>&1; then
    driver_name=uio_pci_generic
elif [[ -r "$rootdir/dpdk/build/kmod/igb_uio.ko" ]]; then
    driver_path="$rootdir/dpdk/build/kmod/igb_uio.ko"
    driver_name="igb_uio"
    modprobe uio
    echo "WARNING: uio_pci_generic not detected - using $driver_name"
else
    echo "No valid drivers found [vfio-pci, uio_pci_generic, igb_uio]. Please either enable the vfio-pci or uio_pci_generic"
    echo "kernel modules, or have SPDK build the igb_uio driver by running ./configure --with-igb-uio-driver and recompiling."
    return 1
fi

# modprobe assumes the directory of the module. If the user passes in a path, we should use insmod
if [[ -n "$driver_path" ]]; then
    insmod $driver_path || true
else
    modprobe $driver_name
fi

cd $rootdir
make udev_uninstall
sleep 5
cd -

$spdkdir/setup.sh reset
sleep 10

for bdf in $(iter_all_pci_class_code 01 08 02); do
		blknames=()
		if ! pci_can_use $bdf; then
			pci_dev_echo "$bdf" "Skipping un-whitelisted NVMe controller at $bdf"
			continue
		fi

		mount=false
		for blkname in $(get_nvme_name_from_bdf $bdf); do
			mountpoints=$(lsblk /dev/$blkname --output MOUNTPOINT -n | wc -w)
			if [ "$mountpoints" != "0" ]; 
            then
				mount=true
				blknames+=($blkname)
            else
                dd if=/dev/zero of=/dev/$blkname bs=512 count=512 oflag=direct
			    echo "$bdf $blkname : MBR Data removed"
			fi
		done
done

sudo $rootdir/script/setup_env.sh

cd $rootdir
make udev_install
cd -