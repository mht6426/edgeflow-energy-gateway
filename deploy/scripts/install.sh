#!/usr/bin/env sh
set -eu

BIN_PATH="${1:-build/edgeflow}"
CFG_PATH="${2:-configs/gateway.prod.json}"

install -d /usr/local/bin
install -d /etc/edgeflow
install -d /var/lib/edgeflow
install -d /var/log/edgeflow

install -m 0755 "$BIN_PATH" /usr/local/bin/edgeflow
install -m 0644 "$CFG_PATH" /etc/edgeflow/gateway.json
install -m 0644 deploy/systemd/edgeflow.service /etc/systemd/system/edgeflow.service

systemctl daemon-reload
systemctl enable edgeflow

echo "installed edgeflow. start with: systemctl start edgeflow"
