#!/bin/bash

if [ -z "$(lsmod | grep nvme_rdma)" ]; then
	modprobe ib_cm
	modprobe ib_core
	# Newer kernels do not have the ib_ucm module
	modprobe ib_ucm || true
	modprobe ib_umad
	modprobe ib_uverbs
	modprobe iw_cm
	modprobe rdma_cm
	modprobe rdma_ucm

	modprobe mlx5_core
	modprobe mlx5_ib
	modprobe nvme_rdma
	modprobe nvme_tcp
fi