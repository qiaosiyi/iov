#!/bin/sh

rm -r demoCA
mkdir demoCA
cd demoCA
mkdir certs newcerts private crl
touch index.txt serial
echo '01' > serial

cd ..

# create ca
openssl req -new -x509 -keyout ca.key -out ca.crt -config openssl.cnf

#crete server.crt
openssl genrsa -des3 -out server.key 2048
openssl req -new -key server.key -out server.csr -config openssl.cnf
openssl ca -in server.csr -out server.crt -cert ca.crt -keyfile ca.key -config openssl.cnf

#create client.crt
openssl genrsa -des3 -out client.key 2048
openssl req -new -key client.key -out client.csr -config openssl.cnf
openssl ca -in client.csr -out client.crt -cert ca.crt -keyfile ca.key -config openssl.cnf