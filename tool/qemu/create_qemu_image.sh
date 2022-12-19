#!/usr/bin/bash

if [ $# -ne 3 ]
then
	echo -e "Syntax: \n	$0 [image_dir] [vm_image] [iso_image]\nExample:\n	$0 ~/workspace/qemu/vm1 pos-ubuntu1804.qcow2 ubuntu-18.04.6-live-server-amd64.iso"
	exit 1
fi

image_dir=$1
vm_image=$2
install_img=$3

if [ ! -f ${image_dir}/${vm_image} ]; then
	echo "# Install ${vm_image}"
	mkdir -p ${image_dir}
	qemu-img create -f qcow2 ${image_dir}/${vm_image} 128G
	qemu-system-x86_64 \
		-cpu qemu64,+ssse3,+sse4.1,+sse4.2 \
		-m 8G \
		-enable-kvm \
		-drive if=virtio,file=${image_dir}/${vm_image},cache=none \
		-drive if=pflash,format=raw,readonly=yes,file=/usr/share/OVMF/OVMF_CODE.fd \
		-drive if=pflash,format=raw,file=/usr/share/OVMF/OVMF_VARS.fd \
		-cdrom ${install_img}
fi