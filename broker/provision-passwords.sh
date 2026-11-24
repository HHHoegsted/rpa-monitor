#!/bin/sh

# Generate the Mosquitto password file from environment variables.
# Existing password hashes are replaced on successful provisioning.

set -eu
umask 077

: "${MQTT_PUBLISHER_USERNAME:?Publisher username required}"
: "${MQTT_PUBLISHER_PASSWORD:?Publisher password required}"
: "${MQTT_COLLECTOR_USERNAME:?Collector username required}"
: "${MQTT_COLLECTOR_PASSWORD:?Collector password required}"

if [ "$MQTT_PUBLISHER_USERNAME" != "monitor_publisher" ] || \
   [ "$MQTT_COLLECTOR_USERNAME" != "monitor_collector" ]; then
    echo "MQTT usernames do not match the configured ACL."
    exit 1
fi

PASSWORD_DIR="/provisioned"

mkdir -p "$PASSWORD_DIR"

TEMP_FILE="$(mktemp "$PASSWORD_DIR/passwords.XXXXXX")"

trap 'rm -f "$TEMP_FILE"' EXIT

mosquitto_passwd -b -c "$TEMP_FILE" \
    "$MQTT_PUBLISHER_USERNAME" \
    "$MQTT_PUBLISHER_PASSWORD"

mosquitto_passwd -b "$TEMP_FILE" \
    "$MQTT_COLLECTOR_USERNAME" \
    "$MQTT_COLLECTOR_PASSWORD"

chown 1883:1883 "$TEMP_FILE"
chmod 600 "$TEMP_FILE"

mv -f "$TEMP_FILE" "$PASSWORD_DIR/passwords"

echo "MQTT password provisioning completed."