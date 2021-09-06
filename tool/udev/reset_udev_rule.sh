#!/usr/bin/env bash

POS_ROOT_DIR=$(readlink -f $(dirname $0))/../..
UDEV_DIR="/etc/udev/rules.d"
UDEV_FILE="${UDEV_DIR}/99-custom-nvme.rules"

rm ${UDEV_FILE};
udevadm control --reload-rules && udevadm trigger
${POS_ROOT_DIR}/tool/udev/generate_udev_rule.sh
cp ${POS_ROOT_DIR}/tool/udev/99-custom-nvme.rules ${UDEV_FILE}
udevadm control --reload-rules && udevadm trigger