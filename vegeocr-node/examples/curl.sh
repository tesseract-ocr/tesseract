#!/bin/sh
# Runnable walkthrough of every feature. Usage:  sh examples/curl.sh [image.png]   (server on $URL, default :4000)
URL="${URL:-http://localhost:4000}"
IMG="${1:-bench/sample-page.png}"
hr() { printf '\n\033[1m== %s\033[0m\n' "$1"; }

hr "1. Compression negotiation (server picks zstd > br > gzip when the client rates them equally)"
for AE in "zstd, br, gzip" "gzip, deflate, br" "br;q=0.4, gzip;q=0.9" "identity"; do
  printf '%-26s -> ' "$AE"
  curl -s -o /dev/null -D - -H "Accept-Encoding: $AE" -F file=@"$IMG" "$URL/api/ocr" | tr -d '\r' | grep -iE '^(content-encoding|content-length)' | tr '\n' ' '; echo
done

hr "2. OCR result: size on the wire"
curl -s -H 'Accept-Encoding: br' -F file=@"$IMG" "$URL/api/ocr" -o /dev/null -w 'br:       %{size_download} bytes downloaded\n'
curl -s -H 'Accept-Encoding: identity' -F file=@"$IMG" "$URL/api/ocr" -o /dev/null -w 'identity: %{size_download} bytes downloaded\n'

hr "3. Caching + ETag + 304 (same bytes => same cache key; OCR runs once)"
curl -s -o /dev/null -D /tmp/h1 -H 'Accept-Encoding: br' -F file=@"$IMG" "$URL/api/ocr"; grep -iE '^(x-cache|x-cache-variant|etag)' /tmp/h1 | tr -d '\r'
ETAG=$(grep -i '^etag' /tmp/h1 | cut -d' ' -f2- | tr -d '\r')
printf 'with If-None-Match: '; curl -s -o /dev/null -w '%{http_code}\n' -H 'Accept-Encoding: br' -H "If-None-Match: $ETAG" -F file=@"$IMG" "$URL/api/ocr"

hr "4. Queue + job status + SSE (force async; PDFs and big files do this automatically)"
# A fresh file each time so this isn't answered from the cache.
cp "$IMG" /tmp/async-demo.png; head -c 16 /dev/urandom >> /tmp/async-demo.png
JOB=$(curl -s -F file=@/tmp/async-demo.png -F mode=async "$URL/api/ocr" | sed -E 's/.*"id":"([^"]+)".*/\1/'); echo "job: $JOB"
curl -s "$URL/api/ocr/$JOB"; echo
curl -sN --max-time 60 "$URL/api/ocr/$JOB/events"
printf 'result: '; curl -s -o /dev/null -w '%{http_code}, %{size_download} bytes (zstd)\n' -H 'Accept-Encoding: zstd' "$URL/api/ocr/$JOB"

hr "5. Upload validation (magic bytes, not Content-Type)"
echo 'not an image' > /tmp/fake.png
curl -s -F "file=@/tmp/fake.png;type=image/png" "$URL/api/ocr"; echo
curl -s -F "other=@$IMG" "$URL/api/ocr"; echo
curl -s -F file=@"$IMG" -F lang=klingon "$URL/api/ocr"; echo

hr "6. Compressed request body (gzip) + decompression-bomb protection"
B="----curlboundary"
{ printf -- "--$B\r\nContent-Disposition: form-data; name=\"file\"; filename=\"p.png\"\r\nContent-Type: image/png\r\n\r\n"; cat "$IMG"; printf -- "\r\n--$B--\r\n"; } | gzip -c > /tmp/body.gz
curl -s -o /dev/null -w 'gzip upload: %{http_code}\n' -H "Content-Type: multipart/form-data; boundary=$B" -H 'Content-Encoding: gzip' --data-binary @/tmp/body.gz "$URL/api/ocr"
head -c 200000000 /dev/zero | gzip -9 > /tmp/bomb.gz
printf 'bomb (%s bytes on the wire, 200 MB inflated): ' "$(wc -c < /tmp/bomb.gz | tr -d ' ')"
curl -s -H "Content-Type: multipart/form-data; boundary=$B" -H 'Content-Encoding: gzip' --data-binary @/tmp/bomb.gz "$URL/api/ocr"; echo

hr "7. Security headers, CORS, rate limiting"
curl -sI "$URL/health" | tr -d '\r' | grep -iE '^(strict-transport|x-content-type|x-frame|content-security|x-request-id)'
printf 'allowed origin : '; curl -s -o /dev/null -D - -H 'Origin: http://localhost:3000' "$URL/health" | tr -d '\r' | grep -i '^access-control-allow-origin' || echo '(none)'
printf 'foreign origin : '; curl -s -o /dev/null -D - -H 'Origin: https://evil.example' "$URL/health" | tr -d '\r' | grep -i '^access-control-allow-origin' || echo '(none: a browser will block it)'

hr "8. Health + Prometheus metrics"
curl -s "$URL/health"; echo
curl -s "$URL/metrics" | grep -E '^(ocr_cache_requests_total|ocr_queue_depth|http_compression_ratio_(sum|count))'
