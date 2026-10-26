#!/usr/bin/env bash
# Build the stack for linux/amd64 on this machine, ship the images to the server, and (re)start it.
#   deploy/deploy.sh ec2-user@HOST /path/to/key.pem [--skip-build]
# First time on a fresh server, also run:  ssh -i KEY USER@HOST 'sudo bash -s' < deploy/server-setup.sh
# and create ~/vegeocr/.env on the server from .env.prod.example (real secrets, PUBLIC_*_URL = the address people will use).
set -euo pipefail
TARGET="${1:?usage: deploy.sh user@host key.pem [--skip-build]}"; KEY="${2:?usage: deploy.sh user@host key.pem [--skip-build]}"
cd "$(dirname "$0")/.."
SSH=(ssh -i "$KEY" -o BatchMode=yes -o ServerAliveInterval=15 "$TARGET")

echo "== reading PUBLIC_API_URL from the server's .env (it is baked into the web build; empty = same origin)"
"${SSH[@]}" "grep -q '^PUBLIC_API_URL=' ~/vegeocr/.env" || { echo "PUBLIC_API_URL= line missing in ~/vegeocr/.env on the server"; exit 1; }
PUBLIC_API_URL="$("${SSH[@]}" "grep '^PUBLIC_API_URL=' ~/vegeocr/.env | cut -d= -f2-")"
echo "   '${PUBLIC_API_URL}'"

if [ "${3:-}" != "--skip-build" ]; then
  echo "== building linux/amd64 images (the first build compiles Tesseract: slow under emulation, cached afterwards)"
  # Build-time interpolation only: the real secrets live in the server's .env and are never baked into an image.
  PUBLIC_API_URL="$PUBLIC_API_URL" PUBLIC_WEB_URL=http://build POSTGRES_PASSWORD=x JWT_SECRET=x ADMIN_EMAIL=x ADMIN_PASSWORD=x \
    docker compose -f docker-compose.prod.yml build
fi

echo "== shipping images (streamed, no temp files: the server disk is small)"
docker save vegeocr/backend:prod vegeocr/frontend:prod vegeocr/edge:prod | gzip -1 | "${SSH[@]}" 'gunzip | sudo docker load'
scp -i "$KEY" -q docker-compose.prod.yml "$TARGET":~/vegeocr/docker-compose.yml

echo "== (re)starting"
"${SSH[@]}" 'cd ~/vegeocr && sudo docker compose up -d --no-build --remove-orphans && sudo docker image prune -f >/dev/null'
for i in $(seq 1 40); do
  S="$("${SSH[@]}" "curl -s -o /dev/null -w '%{http_code}' http://localhost/health" || true)"
  [ "$S" = "200" ] && break; sleep 3
done
"${SSH[@]}" 'cd ~/vegeocr && sudo docker compose ps --format "table {{.Service}}\t{{.Status}}"'
echo "health through Caddy: HTTP $S"
[ "$S" = "200" ]
