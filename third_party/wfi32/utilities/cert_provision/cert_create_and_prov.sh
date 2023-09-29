#!/bin/bash

pai_cert_pem_filename=$1
pai_key_pem_filename=$2

python3 extract_pk.py

pk_filename=$(ls public_* | head -n 1)

echo "filename = $pk_filename"

sn=$(echo $pk_filename | cut -d "-" -f 2 | cut -d "." -f 1)
dac_filename_prefix="Chip-DAC-Cert-"

dac_pem_filename=${dac_filename_prefix}${sn}.pem
dac_der_filename=${dac_filename_prefix}${sn}.der

./chip-cert gen-att-cert --type d --subject-cn "Matter Development DAC 01" --key $pk_filename --subject-vid FFF1 --subject-pid 8001 --valid-from "2022-10-15 14:00:00" --lifetime 7305 --ca-key $pai_key_pem_filename --ca-cert $pai_cert_pem_filename --out $dac_pem_filename

openssl x509 -outform der -in $dac_pem_filename -out $dac_der_filename

python3 dev_provision.py $dac_der_filename


if [ ! -d "archieve" ]; then
    mkdir "archieve"
fi

mv "$pk_filename" "archieve/"
mv "$dac_pem_filename" "archieve/"
mv "$dac_der_filename" "archieve/"
