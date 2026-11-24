#!/bin/sh

# Generate a dedicated CA and a signed MQTT broker certificate.
#
# Arguments:
#   $1 - Broker common name.
#   $2 - Subject alternative names.
#
# Existing certificate material will not be overwritten.

set -eu
umask 077

BROKER_NAME="${1:?Broker name required}"
BROKER_SAN="${2:?Subject alternative names required}"

SECRETS_DIR="/broker/secrets"

mkdir -p "$SECRETS_DIR"
cd "$SECRETS_DIR"

if [ -e ca.key ] || [ -e ca.crt ] || \
   [ -e server.key ] || [ -e server.crt ]; then
    echo "Certificate material already exists. Aborting."
    exit 1
fi

echo "Generating Certificate Authority..."

openssl req -x509 -newkey rsa:3072 -sha256 \
    -days 3650 \
    -keyout ca.key \
    -out ca.crt \
    -subj "/CN=RPA Drift CA" \
    -addext "basicConstraints=critical,CA:TRUE" \
    -addext "keyUsage=critical,keyCertSign,cRLSign" \
    -addext "subjectKeyIdentifier=hash"

echo "Generating broker private key and signing request..."

openssl req -new -newkey rsa:2048 -noenc \
    -keyout server.key \
    -out server.csr \
    -subj "/CN=$BROKER_NAME"

cat > server.ext <<EOF
basicConstraints=critical,CA:FALSE
keyUsage=critical,digitalSignature,keyEncipherment
extendedKeyUsage=serverAuth
subjectAltName=$BROKER_SAN
subjectKeyIdentifier=hash
authorityKeyIdentifier=keyid,issuer
EOF

echo "Signing broker certificate..."

openssl x509 -req \
    -in server.csr \
    -CA ca.crt \
    -CAkey ca.key \
    -CAcreateserial \
    -out server.crt \
    -days 365 \
    -sha256 \
    -extfile server.ext

echo "Configuring certificate permissions..."

chmod 644 ca.crt server.crt
chmod 600 ca.key server.key

echo "Verifying certificate chain..."

openssl verify -purpose sslserver \
    -CAfile ca.crt server.crt

openssl x509 -in server.crt -noout -ext subjectAltName

echo "TLS certificate provisioning completed."