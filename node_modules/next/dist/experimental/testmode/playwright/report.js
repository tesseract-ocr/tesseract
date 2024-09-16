"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "reportFetch", {
    enumerable: true,
    get: function() {
        return reportFetch;
    }
});
const _step = require("./step");
async function parseBody(r) {
    const contentType = r.headers.get("content-type");
    let error;
    let text;
    let json;
    let formData;
    let buffer;
    if (contentType == null ? void 0 : contentType.includes("text")) {
        try {
            text = await r.text();
        } catch (e) {
            error = "failed to parse text";
        }
    } else if (contentType == null ? void 0 : contentType.includes("json")) {
        try {
            json = await r.json();
        } catch (e) {
            error = "failed to parse json";
        }
    } else if (contentType == null ? void 0 : contentType.includes("form-data")) {
        try {
            formData = await r.formData();
        } catch (e) {
            error = "failed to parse formData";
        }
    } else {
        try {
            buffer = await r.arrayBuffer();
        } catch (e) {
            error = "failed to parse arrayBuffer";
        }
    }
    return {
        ...error ? {
            error
        } : null,
        ...text ? {
            text
        } : null,
        ...json ? {
            json: JSON.stringify(json)
        } : null,
        ...formData ? {
            formData: JSON.stringify(Array.from(formData))
        } : null,
        ...buffer && buffer.byteLength > 0 ? {
            buffer: `base64;${Buffer.from(buffer).toString("base64")}`
        } : null
    };
}
function parseHeaders(headers) {
    return Object.fromEntries(Array.from(headers).sort(([key1], [key2])=>key1.localeCompare(key2)).map(([key, value])=>{
        return [
            `header.${key}`,
            value
        ];
    }));
}
async function reportFetch(testInfo, req, handler) {
    return (0, _step.step)(testInfo, {
        title: `next.onFetch: ${req.method} ${req.url}`,
        category: "next.onFetch",
        apiName: "next.onFetch",
        params: {
            method: req.method,
            url: req.url,
            ...await parseBody(req.clone()),
            ...parseHeaders(req.headers)
        }
    }, async (complete)=>{
        const res = await handler(req);
        if (res === undefined || res == null) {
            complete({
                error: {
                    message: "unhandled"
                }
            });
        } else if (typeof res === "string" && res !== "continue") {
            complete({
                error: {
                    message: res
                }
            });
        } else {
            let body;
            if (typeof res === "string") {
                body = {
                    response: res
                };
            } else {
                const { status, statusText } = res;
                body = {
                    status,
                    ...statusText ? {
                        statusText
                    } : null,
                    ...await parseBody(res.clone()),
                    ...parseHeaders(res.headers)
                };
            }
            await (0, _step.step)(testInfo, {
                title: `next.onFetch.fulfilled: ${req.method} ${req.url}`,
                category: "next.onFetch",
                apiName: "next.onFetch.fulfilled",
                params: {
                    ...body,
                    "request.url": req.url,
                    "request.method": req.method
                }
            }, async ()=>undefined).catch(()=>undefined);
        }
        return res;
    });
}

//# sourceMappingURL=report.js.map