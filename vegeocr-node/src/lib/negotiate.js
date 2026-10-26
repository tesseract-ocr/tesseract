import accepts from "accepts";
import { ENCODING_ORDER } from "./codecs.js";

/** Parses the q-values out of an Accept-Encoding header: { gzip: 1, br: 0.8, "*": 0.1 }. */
export function parseQ(header = "") {
  const out = {};
  for (const part of String(header).split(",")) {
    const [token, ...params] = part.trim().toLowerCase().split(";");
    if (!token) continue;
    const q = params.map((p) => p.trim()).find((p) => p.startsWith("q="));
    const value = q ? Number.parseFloat(q.slice(2)) : 1;
    out[token.trim()] = Number.isFinite(value) ? value : 1;
  }
  return out;
}

/**
 * Picks the response encoding.
 *
 * `accepts` decides what is *acceptable* (handles `*`, `identity;q=0`, q=0 exclusions). But when the client rates
 * several encodings equally - which is what every browser does ("gzip, deflate, br, zstd") - `accepts` returns the
 * client's *first* one, i.e. gzip. We want OUR preference (zstd > br > gzip) among the equally-rated ones, so we rank
 * by q first and break ties with ENCODING_ORDER.
 */
export function negotiateEncoding(req, available = ENCODING_ORDER) {
  const header = req.headers["accept-encoding"];
  if (header === undefined) return "identity";
  // accepts().encodings() with no argument returns EVERYTHING the client accepts (q > 0); with an argument it returns
  // only the single best match, which would hide the ties we need to break.
  const acceptable = new Set(accepts(req).encodings().map((e) => e.toLowerCase()));
  const q = parseQ(header);
  const rate = (enc) => (enc in q ? q[enc] : "*" in q ? q["*"] : 0);
  const candidates = available
    .filter((e) => e !== "identity" && (acceptable.has(e) || acceptable.has("*")) && rate(e) > 0)
    .sort((a, b) => rate(b) - rate(a) || ENCODING_ORDER.indexOf(a) - ENCODING_ORDER.indexOf(b));
  return candidates[0] ?? "identity";
}

/** Only text-like payloads are worth compressing. Images, PDFs and archives are already compressed. */
export function isCompressibleType(contentType = "") {
  const t = String(contentType).split(";")[0].trim().toLowerCase();
  return (
    t === "application/json" || t.endsWith("+json") || t.startsWith("text/") && t !== "text/event-stream" ||
    t === "application/xml" || t.endsWith("+xml") || t === "image/svg+xml" ||
    t === "application/javascript" || t === "application/x-ndjson"
  );
}
