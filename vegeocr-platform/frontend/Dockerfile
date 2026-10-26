# Node 22.15+ is needed for the built-in zstd used by scripts/precompress.mjs
FROM node:22-alpine AS build
WORKDIR /app
COPY package*.json ./
RUN npm ci
COPY . .
# The browser calls the API directly, so this must be the PUBLIC url of the API.
ARG NEXT_PUBLIC_API_URL=http://localhost:8000
ENV NEXT_PUBLIC_API_URL=$NEXT_PUBLIC_API_URL
RUN npm run build \
 && node scripts/precompress.mjs .next/static   # max-level .zst/.br/.gz copies for the edge to serve

FROM caddy:2-alpine AS edge
COPY Caddyfile /etc/caddy/Caddyfile
COPY --from=build /app/.next/static /srv/_next/static

FROM node:22-alpine AS runtime
WORKDIR /app
ENV NODE_ENV=production
COPY --from=build /app/.next/standalone ./
COPY --from=build /app/.next/static ./.next/static
USER node
EXPOSE 3000
CMD ["node", "server.js"]
