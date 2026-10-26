# Build context is the REPOSITORY ROOT (the Tesseract source is built here):
#   docker build -f vegeocr-platform/backend/Dockerfile .
FROM debian:bookworm-slim AS tesseract
RUN apt-get update && apt-get install -y --no-install-recommends build-essential cmake pkg-config ca-certificates \
      libleptonica-dev libpango1.0-dev libcairo2-dev libicu-dev libtiff-dev libpng-dev libjpeg-dev \
    && rm -rf /var/lib/apt/lists/*
COPY . /src
RUN cmake -S /src -B /build -DCMAKE_BUILD_TYPE=Release -DBUILD_TRAINING_TOOLS=ON -DCMAKE_INSTALL_PREFIX=/opt/tesseract \
    && cmake --build /build -j"$(nproc)" \
    && cmake --install /build

FROM python:3.12-slim-bookworm
RUN apt-get update && apt-get install -y --no-install-recommends curl \
      liblept5 libpango-1.0-0 libpangocairo-1.0-0 libcairo2 libicu72 libtiff6 libpng16-16 libjpeg62-turbo libgomp1 \
    && rm -rf /var/lib/apt/lists/*
COPY --from=tesseract /opt/tesseract /opt/tesseract
ENV PATH="/opt/tesseract/bin:${PATH}" LD_LIBRARY_PATH="/opt/tesseract/lib" \
    TESSERACT_BIN=/opt/tesseract/bin/tesseract LSTMTRAINING_BIN=/opt/tesseract/bin/lstmtraining \
    COMBINE_TESSDATA_BIN=/opt/tesseract/bin/combine_tessdata STORAGE_DIR=/data/storage BASE_TESSDATA=/data/tessdata \
    PYTHONUNBUFFERED=1
WORKDIR /app
COPY vegeocr-platform/backend/requirements.txt vegeocr-platform/backend/requirements-handwriting.txt ./
RUN pip install --no-cache-dir -r requirements.txt
# OPTIONAL handwriting engine (TrOCR): adds ~1 GB to the image and needs ~2 GB of RAM at run time. Off by default.
#   docker compose build --build-arg INSTALL_HANDWRITING=1     then run with ENABLE_HANDWRITING=1 to download the model on first start
ARG INSTALL_HANDWRITING=0
RUN if [ "$INSTALL_HANDWRITING" = "1" ]; then \
      pip install --no-cache-dir torch --index-url https://download.pytorch.org/whl/cpu \
      && pip install --no-cache-dir -r requirements-handwriting.txt; \
    fi
ENV HF_HOME=/data/hf
COPY vegeocr-platform/backend/app ./app
COPY vegeocr-platform/backend/scripts ./scripts
COPY vegeocr-platform/backend/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh && useradd -m -u 1000 vege && mkdir -p /data && chown vege /data
USER vege
EXPOSE 8000
ENTRYPOINT ["/entrypoint.sh"]
CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000"]
