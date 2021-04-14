IP4=$(ip route get 8.8.8.8 | sed -n '/src/{s/.*src *\([^ ]*\).*/\1/p;q}')

echo $IP4

sed -i "s/localhost/$IP4/" cert.conf

openssl genrsa -out cert.key 2048
openssl req -new -key cert.key -out cert.csr -config cert.conf
openssl req -x509 -days 365 -key cert.key -in cert.csr -out cert.crt -days 365 -config cert.conf -extensions req_ext
