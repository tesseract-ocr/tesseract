const $ = (id) => document.getElementById(id);
const form = $("form"), bar = $("bar"), status = $("status"), out = $("out");

function transferInfo(url) {
  const e = performance.getEntriesByName(new URL(url, location.href).href).pop();
  return e ? ` · ${(e.decodedBodySize / 1024).toFixed(0)} KB decoded, ${(e.encodedBodySize / 1024).toFixed(0)} KB on the wire` : "";
}
function show(url, r) {
  out.textContent = r.text || "(no text found)";
  status.textContent = `${r.pageCount} page(s), confidence ${r.confidence ?? "n/a"}%${transferInfo(url)}`;
  bar.hidden = true;
}
async function fetchResult(url) {
  const res = await fetch(url);
  show(url, await res.json());
}

form.addEventListener("submit", async (e) => {
  e.preventDefault();
  out.textContent = ""; status.textContent = "Uploading…"; bar.hidden = false; bar.removeAttribute("value");
  const res = await fetch("/api/ocr", { method: "POST", body: new FormData(form) });
  const body = await res.json();
  if (!res.ok) { status.textContent = `${body.error.code}: ${body.error.message}`; bar.hidden = true; return; }
  if (res.status === 200) return show("/api/ocr", body);
  // 202: long job - follow it with Server-Sent Events
  status.textContent = "Queued…";
  const es = new EventSource(body.links.events);
  es.addEventListener("progress", (ev) => { const p = JSON.parse(ev.data); bar.max = p.total; bar.value = p.page; status.textContent = `Page ${p.page} of ${p.total}`; });
  es.addEventListener("done", async () => { es.close(); await fetchResult(body.links.self); });
  es.addEventListener("failed", (ev) => { es.close(); status.textContent = JSON.parse(ev.data).error.message; bar.hidden = true; });
  es.onerror = () => status.textContent = "Connection lost, retrying…";
});
