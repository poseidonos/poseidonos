#!/bin/bash

SERVICE_FILE=/etc/systemd/system/poseidon_daemon.service
watchdogdir=$(readlink -f $(dirname $0))
ibofdfiledir=$(readlink -f $(dirname $0))/poseidon_daemon.service
rm ${ibofdfiledir}

echo "[Unit]
Description=poseidon_daemon, poseidonos watchdog daemon

[Service]
User=root
WorkingDirectory=${watchdogdir}/
Type=forking
ExecStart=${watchdogdir}/poseidon_daemon.py -d 2
ExecStop=${watchdogdir}/poseidon_daemon.py -f 1
#Restart=on-failure

[Install]
WantedBy=multi-user.target
Alias=poseidon_daemon.service" >> ${ibofdfiledir}
rm $SERVICE_FILE 2>/dev/null
ln -s ${ibofdfiledir} $SERVICE_FILE
systemctl daemon-reload
systemctl enable poseidon_daemon
systemctl start poseidon_daemon
systemctl status poseidon_daemon

#systemctl stop poseidon_daemon
#systemctl disable poseidon_daemon
