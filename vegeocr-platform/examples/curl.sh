#!/bin/sh
# export VEGEOCR_URL=http://localhost:8000 VEGEOCR_KEY=vocr_xxx
U="${VEGEOCR_URL:-http://localhost:8000}"

# 1. OCR a file (small files answer immediately with 200; large ones answer 202 + a job id)
curl -s -X POST "$U/api/v1/ocr" -H "X-API-Key: $VEGEOCR_KEY" -F "file=@$1" -F "lang=jpn+eng"

# 2. Force the async path, then poll
#    curl -s -X POST "$U/api/v1/ocr" -H "X-API-Key: $VEGEOCR_KEY" -F "file=@$1" -F "async=true"
#    curl -s "$U/api/v1/jobs/JOB_ID" -H "X-API-Key: $VEGEOCR_KEY"

# 3. Send a corrected text line to improve the model (image = ONE line of text)
#    curl -s -X POST "$U/api/v1/training-data" -H "X-API-Key: $VEGEOCR_KEY" -F "image=@line.png" -F "text=請求書" -F "lang=jpn"

# 4. Usage of this key
#    curl -s "$U/api/v1/usage" -H "X-API-Key: $VEGEOCR_KEY"
