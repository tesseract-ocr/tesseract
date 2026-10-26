"""Unit + integration tests for app/middleware/compression.py.   cd backend && .venv/bin/python -m pytest tests/test_compression.py -q"""
import asyncio
import gzip
import json

import brotli
import pytest
import zstandard
from starlette.applications import Starlette
from starlette.middleware import Middleware
from starlette.middleware.cors import CORSMiddleware
from starlette.responses import JSONResponse, PlainTextResponse, Response, StreamingResponse
from starlette.routing import Route

from app.middleware.compression import CompressionMiddleware, build_codecs, is_compressible_type, negotiate, parse_accept_encoding

ALL = ["zstd", "br", "gzip"]


# ---- negotiation ------------------------------------------------------------------------------------------------

@pytest.mark.parametrize("header,expected", [
    (None, "identity"),
    ("", "identity"),
    ("gzip, deflate, br, zstd", "zstd"),            # browser header: equal q, OUR order wins (not the client's first = gzip)
    ("gzip, deflate, br", "br"),
    ("gzip", "gzip"),
    ("zstd;q=0.2, br;q=0.9, gzip;q=0.5", "br"),     # q-values beat server order
    ("br;q=0.5, gzip;q=0.5", "br"),                 # tie -> br before gzip
    ("gzip;q=1, br;q=0.9", "gzip"),
    ("*", "zstd"),
    ("*;q=0.1, gzip", "gzip"),                      # explicit beats wildcard
    ("br;q=0, gzip", "gzip"),                       # q=0 = not allowed
    ("zstd;q=0, br;q=0, gzip;q=0", "identity"),
    ("*;q=0", "identity"),
    ("identity", "identity"),
    ("deflate, compress", "identity"),              # nothing we can produce
    ("GZIP, BR", "br"),                             # case-insensitive
    ("x-gzip", "gzip"),                             # RFC alias
    ("gzip;q=banana, br", "br"),                    # malformed weight: ignore that item
    ("gzip;q=7, br;q=0.5", "gzip"),                 # weights clamp to 1
])
def test_negotiate(header, expected):
    assert negotiate(header, ALL) == expected


def test_negotiate_never_picks_what_the_server_lacks():
    assert negotiate("zstd, br, gzip", ["gzip"]) == "gzip"
    assert negotiate("zstd, br", ["gzip"]) == "identity"
    assert negotiate("gzip, br, zstd", ["br", "gzip"]) == "br"


def test_parse_accept_encoding():
    assert parse_accept_encoding("gzip;q=0.8, br, *;q=0.1") == {"gzip": 0.8, "br": 1.0, "*": 0.1}


@pytest.mark.parametrize("ctype,ok", [
    ("application/json", True), ("application/json; charset=utf-8", True), ("text/html; charset=utf-8", True),
    ("text/plain", True), ("text/css", True), ("application/javascript", True), ("application/xml", True),
    ("image/svg+xml", True), ("application/problem+json", True), ("application/vnd.api+json", True),
    ("image/png", False), ("image/jpeg", False), ("image/webp", False), ("video/mp4", False), ("audio/mpeg", False),
    ("font/woff2", False), ("font/ttf", False), ("application/font-woff", False), ("application/zip", False),
    ("application/gzip", False), ("application/pdf", False), ("application/octet-stream", False),
    ("text/event-stream", False), ("", False),
])
def test_is_compressible_type(ctype, ok):
    assert is_compressible_type(ctype) is ok


# ---- middleware, driven as raw ASGI so we see the exact bytes on the wire ------------------------------------------

BIG = {"rows": [{"i": i, "name": f"item-{i}", "note": "lorem ipsum dolor sit amet"} for i in range(300)]}
BIG_BYTES = JSONResponse(BIG).body  # exactly the bytes Starlette serialises (compact separators)


def make_app(**kw):
    async def big(_): return JSONResponse(BIG)
    async def small(_): return JSONResponse({"ok": True})
    async def text(_): return PlainTextResponse("hello " * 500)
    async def html(_): return Response("<html>" + "<p>x</p>" * 400 + "</html>", media_type="text/html")
    async def png(_): return Response(b"\x89PNG" + bytes(5000), media_type="image/png")
    async def pdf(_): return Response(b"%PDF-" + bytes(5000), media_type="application/pdf")
    async def zipf(_): return Response(bytes(5000), media_type="application/zip")
    async def font(_): return Response(bytes(5000), media_type="font/woff2")
    async def encoded(_): return Response(gzip.compress(BIG_BYTES), media_type="application/json", headers={"Content-Encoding": "gzip"})
    async def notransform(_): return Response(BIG_BYTES, media_type="application/json", headers={"Cache-Control": "no-transform"})
    async def etag(_): return Response(BIG_BYTES, media_type="application/json", headers={"ETag": '"abc123"'})
    async def vary(_): return Response(BIG_BYTES, media_type="application/json", headers={"Vary": "Origin"})
    async def empty(_): return Response(status_code=204)
    async def partial(_): return Response(BIG_BYTES[:2000], status_code=206, media_type="application/json", headers={"Content-Range": "bytes 0-1999/9999"})
    async def boom(_): raise RuntimeError("x")
    async def stream(_):
        async def gen():
            for _i in range(5):
                yield b"data: tick\n\n"
        return StreamingResponse(gen(), media_type="text/event-stream")
    async def chunked(_):
        async def gen():
            for i in range(40):
                yield (f"line {i} " + "y" * 90 + "\n").encode()
        return StreamingResponse(gen(), media_type="text/plain")
    routes = [Route(p, e, methods=["GET", "HEAD"]) for p, e in {
        "/big": big, "/small": small, "/text": text, "/html": html, "/png": png, "/pdf": pdf, "/zip": zipf, "/font": font,
        "/encoded": encoded, "/notransform": notransform, "/etag": etag, "/vary": vary, "/empty": empty, "/partial": partial,
        "/stream": stream, "/chunked": chunked, "/boom": boom}.items()]
    return Starlette(routes=routes, middleware=[
        Middleware(CORSMiddleware, allow_origins=["http://x.test"], allow_methods=["*"]),  # adds Vary: Origin
        Middleware(CompressionMiddleware, **kw)])


def call(app, path, accept=None, method="GET", extra=None):
    """Returns (status, {lowercase header: value}, raw body bytes)."""
    headers = [(b"host", b"t")] + ([(b"accept-encoding", accept.encode())] if accept is not None else [])
    headers += [(k.lower().encode(), v.encode()) for k, v in (extra or {}).items()]
    scope = {"type": "http", "asgi": {"version": "3.0"}, "http_version": "1.1", "method": method, "path": path, "raw_path": path.encode(),
             "query_string": b"", "headers": headers, "server": ("t", 80), "client": ("c", 1), "scheme": "http", "root_path": ""}
    out = {"status": None, "headers": {}, "body": b""}

    async def run():
        delivered = False

        async def receive():
            nonlocal delivered
            if not delivered:
                delivered = True
                return {"type": "http.request", "body": b"", "more_body": False}
            await asyncio.sleep(3600)  # the client stays connected until the response is done (the task is then cancelled)
        async def send(m):
            if m["type"] == "http.response.start":
                out["status"] = m["status"]
                out["headers"] = {k.decode().lower(): v.decode() for k, v in m["headers"]}
            elif m["type"] == "http.response.body":
                out["body"] += m.get("body", b"")
        await app(scope, receive, send)

    asyncio.run(run())
    return out["status"], out["headers"], out["body"]


DECODE = {"gzip": gzip.decompress, "br": brotli.decompress, "zstd": lambda b: zstandard.ZstdDecompressor().decompress(b, max_output_size=10**8)}


@pytest.mark.parametrize("enc", ["zstd", "br", "gzip"])
def test_each_encoding_round_trips_and_shrinks(enc):
    status, h, body = call(make_app(), "/big", enc)
    assert status == 200 and h["content-encoding"] == enc
    assert int(h["content-length"]) == len(body) < len(BIG_BYTES) / 3
    assert DECODE[enc](body) == BIG_BYTES


def test_priority_zstd_over_br_over_gzip():
    assert call(make_app(), "/big", "gzip, deflate, br, zstd")[1]["content-encoding"] == "zstd"
    assert call(make_app(), "/big", "gzip, deflate, br")[1]["content-encoding"] == "br"
    assert call(make_app(), "/big", "gzip, deflate")[1]["content-encoding"] == "gzip"


def test_q_values_and_q0_are_respected():
    assert call(make_app(), "/big", "zstd;q=0.1, gzip;q=0.9")[1]["content-encoding"] == "gzip"
    assert call(make_app(), "/big", "zstd;q=0, br;q=0, gzip;q=0")[1].get("content-encoding") is None
    assert call(make_app(), "/big", "zstd;q=0, br")[1]["content-encoding"] == "br"


def test_no_accept_encoding_means_uncompressed_but_vary_is_set():
    status, h, body = call(make_app(), "/big")
    assert "content-encoding" not in h and body == BIG_BYTES
    assert "accept-encoding" in h["vary"].lower()


def test_vary_is_merged_with_existing_vary():
    _, h, _ = call(make_app(), "/vary", "br")
    assert [t.strip() for t in h["vary"].split(",")] == ["Origin", "Accept-Encoding"]
    _, h, _ = call(make_app(), "/big", "br", extra={"Origin": "http://x.test"})
    tokens = [t.strip().lower() for t in h["vary"].split(",")]
    assert tokens.count("accept-encoding") == 1 and "origin" in tokens


def test_small_responses_are_not_compressed():
    status, h, body = call(make_app(), "/small", "zstd, br, gzip")
    assert "content-encoding" not in h and json.loads(body) == {"ok": True}


def test_threshold_is_configurable():
    assert call(make_app(min_bytes=5), "/small", "gzip")[1].get("content-encoding") is None  # 15 bytes: not worth it
    status, h, body = call(make_app(min_bytes=1), "/text", "gzip")
    assert h["content-encoding"] == "gzip" and DECODE["gzip"](body) == b"hello " * 500


@pytest.mark.parametrize("path", ["/png", "/pdf", "/zip", "/font"])
def test_already_compressed_formats_pass_through_untouched(path):
    status, h, body = call(make_app(), path, "zstd, br, gzip")
    assert status == 200 and "content-encoding" not in h and "vary" not in {k for k in h if "accept-encoding" in h[k].lower()}
    assert len(body) >= 5000


def test_text_html_and_text_plain_are_compressed():
    for p in ("/html", "/text"):
        assert call(make_app(), p, "br")[1]["content-encoding"] == "br"


def test_head_requests_are_not_compressed():
    status, h, body = call(make_app(), "/big", "br", method="HEAD")
    # (A real server strips the body of a HEAD response; what matters is that we never encode it.)
    assert status == 200 and "content-encoding" not in h


def test_already_encoded_and_no_transform_are_left_alone():
    assert call(make_app(), "/encoded", "br")[1]["content-encoding"] == "gzip"
    assert call(make_app(), "/notransform", "br")[1].get("content-encoding") is None


def test_204_206_and_range_requests_are_left_alone():
    assert call(make_app(), "/empty", "br")[0] == 204
    s, h, b = call(make_app(), "/partial", "br")
    assert s == 206 and "content-encoding" not in h and len(b) == 2000
    assert "content-encoding" not in call(make_app(), "/big", "br", extra={"Range": "bytes=0-99"})[1]


def test_etag_gets_a_per_encoding_suffix():
    _, h, _ = call(make_app(), "/etag", "br")
    assert h["etag"] == '"abc123-br"'
    assert call(make_app(), "/etag", "identity")[1]["etag"] == '"abc123"'


def test_event_streams_are_never_buffered_or_compressed():
    status, h, body = call(make_app(), "/stream", "br")
    assert "content-encoding" not in h and body.count(b"tick") == 5


def test_streaming_text_response_is_buffered_then_compressed_correctly():
    status, h, body = call(make_app(), "/chunked", "gzip")
    assert h["content-encoding"] == "gzip"
    assert DECODE["gzip"](body).decode().count("line ") == 40


def test_response_larger_than_buffer_cap_falls_back_to_streaming_uncompressed():
    status, h, body = call(make_app(max_buffer_bytes=1000), "/chunked", "gzip")
    assert status == 200 and "content-encoding" not in h and body.decode().count("line ") == 40


def test_compression_error_falls_back_to_original_bytes():
    def boom(_): raise RuntimeError("codec exploded")
    status, h, body = call(make_app(codecs={"gzip": boom}), "/big", "gzip")
    assert status == 200 and "content-encoding" not in h and body == BIG_BYTES
    assert int(h["content-length"]) == len(BIG_BYTES)


def test_incompressible_payload_is_not_made_bigger():
    import os
    noise = os.urandom(4000)
    app = Starlette(routes=[Route("/n", lambda r: Response(noise, media_type="application/json"))], middleware=[Middleware(CompressionMiddleware)])
    _, h, body = call(app, "/n", "gzip")
    assert body == noise and "content-encoding" not in h


def test_missing_zstd_degrades_gracefully():
    codecs = {k: v for k, v in build_codecs().items() if k != "zstd"}   # simulate a runtime without zstd
    app = make_app(codecs=codecs)
    assert call(app, "/big", "zstd, br, gzip")[1]["content-encoding"] == "br"
    codecs.pop("br")
    assert call(make_app(codecs=codecs), "/big", "zstd, br, gzip")[1]["content-encoding"] == "gzip"
    assert call(make_app(codecs={}), "/big", "zstd, br, gzip")[1].get("content-encoding") is None


def test_disable_env_removes_codecs(monkeypatch):
    monkeypatch.setenv("COMPRESS_DISABLE", "zstd")
    assert call(Starlette(routes=[Route("/b", lambda r: JSONResponse(BIG))], middleware=[Middleware(CompressionMiddleware)]), "/b", "zstd, br, gzip")[1]["content-encoding"] == "br"


def test_unhandled_exception_still_propagates():
    with pytest.raises(RuntimeError):
        call(make_app(), "/boom", "br")


def test_real_codecs_report_availability():
    assert {"gzip", "br", "zstd"} <= set(build_codecs()), "install brotli and zstandard (see requirements.txt)"
