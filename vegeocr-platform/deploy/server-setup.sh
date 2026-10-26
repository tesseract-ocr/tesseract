#!/usr/bin/env bash
# One-time (idempotent) preparation of an Amazon Linux 2023 host for the VegeOCR stack.  Run as root:
#   ssh -i KEY ec2-user@HOST 'sudo bash -s' < deploy/server-setup.sh
set -euo pipefail

echo "== Docker"
if ! command -v docker >/dev/null; then dnf install -y docker; fi
systemctl enable --now docker
usermod -aG docker ec2-user

echo "== Docker Compose plugin"
PLUGIN=/usr/local/lib/docker/cli-plugins/docker-compose
if [ ! -x "$PLUGIN" ]; then
  mkdir -p "$(dirname "$PLUGIN")"
  curl -fsSL "https://github.com/docker/compose/releases/download/v2.29.7/docker-compose-linux-x86_64" -o "$PLUGIN"
  chmod +x "$PLUGIN"
fi

echo "== Swap (this host has ~1 GB of RAM: swap absorbs OCR memory spikes instead of the kernel killing a container)"
if ! swapon --show | grep -q /swapfile; then
  fallocate -l 2G /swapfile 2>/dev/null || dd if=/dev/zero of=/swapfile bs=1M count=2048 status=none
  chmod 600 /swapfile
  mkswap /swapfile >/dev/null
  swapon /swapfile
  grep -q '^/swapfile' /etc/fstab || echo '/swapfile none swap sw 0 0' >> /etc/fstab
fi
echo 'vm.swappiness=20' > /etc/sysctl.d/99-vegeocr.conf
sysctl -q -p /etc/sysctl.d/99-vegeocr.conf

echo "== Result"
docker --version
/usr/local/lib/docker/cli-plugins/docker-compose version
free -m | sed -n '1,3p'
df -h / | tail -1
