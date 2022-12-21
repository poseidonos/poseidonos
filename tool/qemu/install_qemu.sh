#!/bin/bash
# Please run this script as root.

SYSTEM=`uname -s`

if [ -f /etc/debian_version ]; then
    if [ ! -f gost_qemu.tar.gz ]; then
        # Downloads the package lists from the repositories and updates them to get information on the newest versions of packages and their dependencies.
        sudo apt-get update

        # Additional dependencies for QEMU
        sudo apt install -y libvirt-clients libpixman-1-dev libglib2.0-dev qemu-kvm tigervnc-viewer sshpass uml-utilities ovmf python3-pip meson
        sudo pip3 install ninja
        
        echo "Install QEMU v7.1.0"
        git clone -b v7.1.0 https://github.com/qemu/qemu.git

        # Build and Install QEMU
        echo "Build QEMU v7.1.0"
        cd qemu
	    ./configure --with-git-submodules=validate
        make -j ; make install
    fi
else
    echo "Unknown system type."
	exit 1
fi
