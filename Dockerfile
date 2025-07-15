# Base Node.js com Debian
FROM node:18

# Instala Tesseract OCR e Poppler
RUN apt-get update && apt-get install -y \
  tesseract-ocr \
  poppler-utils \
  && apt-get clean

# Diretório de trabalho
WORKDIR /app

# Copia os arquivos do projeto para o container
COPY . .

# Instala dependências do Node.js
RUN npm install

# Usa porta dinâmica (Easypanel vai fornecer via process.env.PORT)
ENV PORT=3000
EXPOSE $PORT

# Inicia o app
CMD ["node", "index.js"]
