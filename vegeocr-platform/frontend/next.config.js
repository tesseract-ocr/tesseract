/** @type {import('next').NextConfig} */
// compress:false because the edge (Caddy) compresses: Next's built-in compression is gzip-only, and it would hide the response
// from Caddy's zstd encoder (an already-encoded response is passed through). Running without the edge? Set it back to true.
module.exports = { reactStrictMode: true, output: "standalone", compress: false };
