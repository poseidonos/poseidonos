#!/bin/bash

if [ $# -ne 3 ]
then
	echo -e "Syntax: \n	$0 [image_dir] [vm_image] [iso_image]\nExample:\n	$0 ~/workspace/qemu/vm1 pos-ubuntu1804.qcow2 ubuntu-18.04.6-live-server-amd64.iso"
	exit 1
fi

image_dir=$1
vm_image=$2
install_img="ubuntu-18.04.6-live-server-amd64.iso"

if [ -f "$install_img" ];then
	echo "ubuntu-18.04.6-live-server-amd64.iso is already exists"
else
	echo "Download ubuntu-18.04.6-live-server-amd64.iso on current directory"
	wget https://releases.ubuntu.com/18.04/ubuntu-18.04.6-live-server-amd64.iso
fi

if [ ! -f ${image_dir}/${vm_image} ]; then
	echo "# Install ${vm_image}"
	mkdir -p ${image_dir}
	qemu-img create -f qcow2 ${image_dir}/${vm_image} 30G
	qemu-system-x86_64 \
		-m 8G \
		-enable-kvm \
		-drive if=virtio,file=${image_dir}/${vm_image},cache=none \
		-cdrom ${install_img}
fi