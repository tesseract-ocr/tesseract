"""HTTP response compression with content negotiation, as a pure ASGI middleware.

* Picks the best encoding the client accepts: zstd > br > gzip > identity, honouring q-values (q=0 = refused).
  Equal q-values (what every browser sends) are broken by OUR order, not the client's.
* Compresses only text-like responses (JSON, HTML, text, XML, SVG, JavaScript). Images, video, audio, fonts, ZIP, PDF and
  anything else are passed through untouched.
* Skips bodies under COMPRESS_MIN_BYTES, HEAD requests, 204/206/304, responses that already carry a Content-Encoding or
  `Cache-Control: no-transform`.
* Fast levels for on-the-fly work (zstd 3, brotli 5, gzip 6); compression runs in a thread, never on the event loop.
* If zstd/brotli aren't installed (or compression fails) it degrades to the next encoding / the original bytes.
"""
from __future__ import annotations

import gzip
import logging
import os
import re
from typing import Callable, Dict, List, Optional, Sequence

from starlette.concurrency import run_in_threadpool
from starlette.datastructures import Headers, MutableHeaders
from starlette.types import ASGIApp, Message, Receive, Scope, Send

log = logging.getLogger("compression")

# Server preference, used to break ties when the client rates encodings equally.
SERVER_ORDER = ("zstd", "br", "gzip")

_COMPRESSIBLE_EXACT = {
    "application/json", "application/xml", "application/javascript", "application/x-javascript",
    "application/x-ndjson", "application/graphql", "image/svg+xml",
}
_NEVER = ("text/event-stream",)  # streams must flush as they go


def is_compressible_type(content_type: str) -> bool:
    """Allow-list, so anything unknown (octet-stream, zip, pdf, fonts, images, audio, video...) is left alone."""
    t = content_type.split(";")[0].strip().lower()
    if not t or t in _NEVER:
        return False
    return t.startswith("text/") or t in _COMPRESSIBLE_EXACT or t.endswith("+json") or t.endswith("+xml")


# ---- negotiation ----------------------------------------------------------------------------------------------

def parse_accept_encoding(header: str) -> Dict[str, float]:
    """'gzip;q=0.8, br, *;q=0.1' -> {'gzip': 0.8, 'br': 1.0, '*': 0.1}. Malformed items are ignored."""
    out: Dict[str, float] = {}
    for part in header.split(","):
        token, *params = [p.strip() for p in part.strip().lower().split(";")]
        if not token:
            continue
        q = 1.0
        for p in params:
            if p.startswith("q="):
                try:
                    q = max(0.0, min(1.0, float(p[2:])))
                except ValueError:
                    q = -1.0  # invalid weight: drop this item
        if q >= 0:
            out.setdefault(token, q)
    if "gzip" not in out and "x-gzip" in out:  # RFC 9110: x-gzip is an alias of gzip
        out["gzip"] = out["x-gzip"]
    return out


def negotiate(accept_encoding: Optional[str], available: Sequence[str]) -> str:
    """Returns one of `available` or 'identity'. No header, or nothing acceptable, means identity."""
    if not accept_encoding:
        return "identity"
    q = parse_accept_encoding(accept_encoding)
    rate = lambda enc: q.get(enc, q.get("*", 0.0))  # noqa: E731 - explicit beats wildcard; unlisted without '*' = refused
    candidates = [e for e in SERVER_ORDER if e in available and rate(e) > 0]
    candidates.sort(key=lambda e: (-rate(e), SERVER_ORDER.index(e)))  # q first, then our order
    return candidates[0] if candidates else "identity"


# ---- codecs ---------------------------------------------------------------------------------------------------

def _env_int(name: str, default: int) -> int:
    try:
        return int(os.environ.get(name, default))
    except ValueError:
        return default


def build_codecs(levels: Optional[Dict[str, int]] = None, disabled: Sequence[str] = ()) -> Dict[str, Callable[[bytes], bytes]]:
    """name -> compress(bytes). Only encodings this runtime can actually produce are present."""
    lv = {"zstd": _env_int("COMPRESS_ZSTD_LEVEL", 3), "br": _env_int("COMPRESS_BROTLI_QUALITY", 5),
          "gzip": _env_int("COMPRESS_GZIP_LEVEL", 6), **(levels or {})}
    codecs: Dict[str, Callable[[bytes], bytes]] = {"gzip": lambda b: gzip.compress(b, compresslevel=lv["gzip"], mtime=0)}

    try:  # Python 3.14+ ships zstd in the standard library
        from compression import zstd as _std  # type: ignore[import-not-found]
        codecs["zstd"] = lambda b: _std.compress(b, level=lv["zstd"])
    except ImportError:
        try:
            import zstandard

            # A compressor object isn't safe to share between threads, so make one per call (cheap).
            codecs["zstd"] = lambda b: zstandard.ZstdCompressor(level=lv["zstd"]).compress(b)
        except ImportError:
            log.warning("zstd unavailable (pip install zstandard); falling back to brotli/gzip")
    try:
        try:
            import brotli
        except ImportError:
            import brotlicffi as brotli  # type: ignore[no-redef]
        codecs["br"] = lambda b: brotli.compress(b, mode=brotli.MODE_TEXT, quality=lv["br"])
    except ImportError:
        log.warning("brotli unavailable (pip install brotli); falling back to gzip")
    for name in disabled:
        codecs.pop(name.strip(), None)
    return codecs


# ---- middleware -----------------------------------------------------------------------------------------------

class CompressionMiddleware:
    def __init__(self, app: ASGIApp, min_bytes: Optional[int] = None, max_buffer_bytes: Optional[int] = None,
                 codecs: Optional[Dict[str, Callable[[bytes], bytes]]] = None) -> None:
        self.app = app
        self.min_bytes = min_bytes if min_bytes is not None else _env_int("COMPRESS_MIN_BYTES", 1024)
        self.max_buffer = max_buffer_bytes if max_buffer_bytes is not None else _env_int("COMPRESS_MAX_BUFFER_MB", 16) * 1024 * 1024
        disabled = [s for s in os.environ.get("COMPRESS_DISABLE", "").split(",") if s]
        self.codecs = codecs if codecs is not None else build_codecs(disabled=disabled)

    async def __call__(self, scope: Scope, receive: Receive, send: Send) -> None:
        if scope["type"] != "http":
            return await self.app(scope, receive, send)

        request_headers = Headers(scope=scope)
        is_head = scope["method"] == "HEAD"
        chosen = "identity" if is_head else negotiate(request_headers.get("accept-encoding"), list(self.codecs))
        if "range" in request_headers:  # partial content must stay byte-exact
            chosen = "identity"

        start: Optional[Message] = None
        chunks: List[bytes] = []
        size = 0
        passthrough = False

        async def flush_uncompressed(final: bool = False) -> None:
            nonlocal passthrough
            assert start is not None
            await send(start)
            for c in chunks:
                await send({"type": "http.response.body", "body": c, "more_body": True})
            chunks.clear()
            passthrough = True
            if final:
                await send({"type": "http.response.body", "body": b"", "more_body": False})

        async def send_wrapper(message: Message) -> None:
            nonlocal start, size, passthrough
            if passthrough:
                return await send(message)

            if message["type"] == "http.response.start":
                headers = MutableHeaders(raw=list(message["headers"]))
                status = message["status"]
                ctype = headers.get("content-type", "")
                if (not is_compressible_type(ctype) or headers.get("content-encoding") or "content-range" in headers
                        or status in (204, 206, 304) or status < 200
                        or "no-transform" in headers.get("cache-control", "").lower()):
                    passthrough = True
                    return await send(message)
                _add_vary(headers, "Accept-Encoding")  # this URL's bytes now depend on Accept-Encoding
                message = {**message, "headers": headers.raw}
                length = headers.get("content-length")
                if chosen == "identity" or (length is not None and (int(length) < self.min_bytes or int(length) > self.max_buffer)):
                    passthrough = True
                    return await send(message)
                start = message  # hold the headers until we know the body size
                return

            if message["type"] != "http.response.body" or start is None:
                return await send(message)

            body = message.get("body", b"")
            more = message.get("more_body", False)
            chunks.append(body)
            size += len(body)
            if size > self.max_buffer:  # too big to buffer: send what we have uncompressed and stream the rest
                await flush_uncompressed(final=not more)
                return
            if more:
                return

            data = b"".join(chunks)
            out, encoding = data, "identity"
            if len(data) >= self.min_bytes:
                try:
                    packed = await run_in_threadpool(self.codecs[chosen], data)  # never block the event loop
                    if len(packed) < len(data):
                        out, encoding = packed, chosen
                except Exception as exc:  # noqa: BLE001 - serve the original bytes rather than fail the request
                    log.warning("compression with %s failed (%s); sending uncompressed", chosen, exc)
            headers = MutableHeaders(raw=list(start["headers"]))
            if encoding != "identity":
                headers["content-encoding"] = encoding
                etag = headers.get("etag")
                if etag:  # a different representation needs a different validator
                    headers["etag"] = re.sub(r'"$', f'-{encoding}"', etag)
            headers["content-length"] = str(len(out))
            await send({**start, "headers": headers.raw})
            await send({"type": "http.response.body", "body": out, "more_body": False})
            chunks.clear()

        await self.app(scope, receive, send_wrapper)


def _add_vary(headers: MutableHeaders, token: str) -> None:
    existing = headers.get("vary", "")
    if token.lower() not in [t.strip().lower() for t in existing.split(",")]:
        headers["vary"] = f"{existing}, {token}" if existing else token
