import { isResSent } from "../shared/lib/utils";
import { generateETag } from "./lib/etag";
import fresh from "next/dist/compiled/fresh";
import { formatRevalidate } from "./lib/revalidate";
import { RSC_CONTENT_TYPE_HEADER } from "../client/components/app-router-headers";
export function sendEtagResponse(req, res, etag) {
    if (etag) {
        /**
     * The server generating a 304 response MUST generate any of the
     * following header fields that would have been sent in a 200 (OK)
     * response to the same request: Cache-Control, Content-Location, Date,
     * ETag, Expires, and Vary. https://tools.ietf.org/html/rfc7232#section-4.1
     */ res.setHeader("ETag", etag);
    }
    if (fresh(req.headers, {
        etag
    })) {
        res.statusCode = 304;
        res.end();
        return true;
    }
    return false;
}
export async function sendRenderResult({ req, res, result, type, generateEtags, poweredByHeader, revalidate, swrDelta }) {
    if (isResSent(res)) {
        return;
    }
    if (poweredByHeader && type === "html") {
        res.setHeader("X-Powered-By", "Next.js");
    }
    // If cache control is already set on the response we don't
    // override it to allow users to customize it via next.config
    if (typeof revalidate !== "undefined" && !res.getHeader("Cache-Control")) {
        res.setHeader("Cache-Control", formatRevalidate({
            revalidate,
            swrDelta
        }));
    }
    const payload = result.isDynamic ? null : result.toUnchunkedString();
    if (payload !== null) {
        let etagPayload = payload;
        if (type === "rsc") {
            // ensure etag generation is deterministic as
            // ordering can differ even if underlying content
            // does not differ
            etagPayload = payload.split("\n").sort().join("\n");
        } else if (type === "html" && payload.includes("__next_f")) {
            const { parse } = require("next/dist/compiled/node-html-parser");
            try {
                var _root_querySelector;
                // Parse the HTML
                let root = parse(payload);
                // Get script tags in the body element
                let scriptTags = (_root_querySelector = root.querySelector("body")) == null ? void 0 : _root_querySelector.querySelectorAll("script").filter((node)=>{
                    var _node_innerHTML;
                    return !node.hasAttribute("src") && ((_node_innerHTML = node.innerHTML) == null ? void 0 : _node_innerHTML.includes("__next_f"));
                });
                // Sort the script tags by their inner text
                scriptTags == null ? void 0 : scriptTags.sort((a, b)=>a.innerHTML.localeCompare(b.innerHTML));
                // Remove the original script tags
                scriptTags == null ? void 0 : scriptTags.forEach((script)=>script.remove());
                // Append the sorted script tags to the body
                scriptTags == null ? void 0 : scriptTags.forEach((script)=>{
                    var _root_querySelector;
                    return (_root_querySelector = root.querySelector("body")) == null ? void 0 : _root_querySelector.appendChild(script);
                });
                // Stringify back to HTML
                etagPayload = root.toString();
            } catch (err) {
                console.error(`Error parsing HTML payload`, err);
            }
        }
        const etag = generateEtags ? generateETag(etagPayload) : undefined;
        if (sendEtagResponse(req, res, etag)) {
            return;
        }
    }
    if (!res.getHeader("Content-Type")) {
        res.setHeader("Content-Type", result.contentType ? result.contentType : type === "rsc" ? RSC_CONTENT_TYPE_HEADER : type === "json" ? "application/json" : "text/html; charset=utf-8");
    }
    if (payload) {
        res.setHeader("Content-Length", Buffer.byteLength(payload));
    }
    if (req.method === "HEAD") {
        res.end(null);
        return;
    }
    if (payload !== null) {
        res.end(payload);
        return;
    }
    // Pipe the render result to the response after we get a writer for it.
    await result.pipeToNodeResponse(res);
}

//# sourceMappingURL=send-payload.js.map