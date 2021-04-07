#!/bin/bash

SERVICE_FILE=/etc/systemd/system/ibofd.service
watchdogdir=$(readlink -f $(dirname $0))
ibofdfiledir=$(readlink -f $(dirname $0))/ibofd.service
rm ${ibofdfiledir}

echo "[Unit]
Description=ibofd, ibofos watchdog daemon

[Service]
User=root
WorkingDirectory=${watchdogdir}/
Type=forking
ExecStart=${watchdogdir}/ibofd.py -d 2
ExecStop=${watchdogdir}/ibofd.py -f 1
#Restart=on-failure

[Install]
WantedBy=multi-user.target
Alias=ibofd.service" >> ${ibofdfiledir}
rm $SERVICE_FILE 2>/dev/null
ln -s ${ibofdfiledir} $SERVICE_FILE
systemctl daemon-reload
systemctl enable ibofd
systemctl start ibofd
systemctl status ibofd

#systemctl stop ibofd
#systemctl disable ibofd
