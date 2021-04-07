#
#
#/bin/bash
#
# Note : tune tcp performance


check(){
        sysctl -a | grep net.core.rmem_default
        sysctl -a | grep net.core.rmem_max
        sysctl -a | grep net.core.wmem_default
        sysctl -a | grep net.core.wmem_max
        sysctl -a | grep net.ipv4.tcp_wmem
        sysctl -a | grep net.ipv4.tcp_rmem
        sysctl -a | grep net.ipv4.tcp_mtu_probing
}

min(){
        sysctl -w net.core.rmem_default="212992"
        sysctl -w net.core.rmem_max="212992"
        sysctl -w net.core.wmem_default="212992"
        sysctl -w net.core.wmem_max="212992"
        sysctl -w net.ipv4.tcp_wmem="4096 16384 4194304"
        sysctl -w net.ipv4.tcp_rmem="4096 131072 6291456"
        sysctl -w net.ipv4.tcp_mtu_probing="0"
	sysctl -w net.ipv4.tcp_window_scaling="0"
	sysctl -w net.ipv4.tcp_slow_start_after_idle="1"
}


max(){
        sysctl -w net.core.rmem_default="268435456"
        sysctl -w net.core.rmem_max="268435456"
        sysctl -w net.core.wmem_default="268435456"
        sysctl -w net.core.wmem_max="268435456"
        sysctl -w net.ipv4.tcp_wmem="4096 65536 134217728"
        sysctl -w net.ipv4.tcp_rmem="4096 131072 134217728"
        sysctl -w net.ipv4.tcp_mtu_probing="1"
	sysctl -w net.ipv4.tcp_window_scaling="1"
	sysctl -w net.ipv4.tcp_slow_start_after_idle="0"
}

if [ "$1" = "max" ]; then
        echo "set tcp performance mode"
        max
        check
elif [ "$1" = "min" ]; then
        echo "set tcp default mode"
        min
        check
elif [ "$1" = "check" ]; then
        check
else
        echo "Usage: tcp_tune.sh {max | min | check}"
fi

