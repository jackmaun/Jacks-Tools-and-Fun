@echo off
set OPENSSL_PATH="C:\Program Files\OpenSSL-Win64\bin\openssl.exe"

echo Generating private key...
%OPENSSL_PATH% genrsa -out server.key 2048

echo Generating certificate...
%OPENSSL_PATH% req -x509 -new -nodes -key server.key -sha256 -days 1024 -out server.crt

echo Done!
pause