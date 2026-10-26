/**
 * ETags are derived from the content hash (+ language + preprocessing version) and include the content-coding,
 * because the same JSON in br vs identity are different representations and must not share a validator.
 */
export const makeEtag = (cacheKey, encoding) => `"${cacheKey.replace(/[^\w.-]/g, "_")}-${encoding}"`;

/** Weak-comparison If-None-Match matching (RFC 9110 §13.1.2), including `*` and comma lists. */
export function etagMatches(header, etag) {
  if (!header) return false;
  if (header.trim() === "*") return true;
  const strip = (t) => t.trim().replace(/^W\//, "");
  return header.split(",").some((t) => strip(t) === strip(etag));
}
