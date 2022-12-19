#!/bin/bash
# Please run this script as root.

SYSTEM=`uname -s`

if [ -f /etc/debian_version ]; then
    if [ ! -f gost_qemu.tar.gz ]; then
        # Additional dependencies for QEMU
        sudo apt install -y libvirt-clients libpixman-1-dev libglib2.0-dev qemu-kvm tigervnc-viewer sshpass uml-utilities ovmf
        echo "Install QEMU v7.1.0"
        git clone -b v7.1.0 https://github.com/qemu/qemu.git

        # Build and Install QEMU
        echo "Build QEMU v7.1.0"
        cd qemu
        ./configure
        make -j ; make install
    fi
else
    echo "Unknown system type."
	exit 1
fi
