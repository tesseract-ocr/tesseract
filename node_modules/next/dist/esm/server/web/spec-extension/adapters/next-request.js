import { getRequestMeta } from "../../../request-meta";
import { fromNodeOutgoingHttpHeaders } from "../../utils";
import { NextRequest } from "../request";
export const ResponseAbortedName = "ResponseAborted";
export class ResponseAborted extends Error {
    constructor(...args){
        super(...args);
        this.name = ResponseAbortedName;
    }
}
/**
 * Creates an AbortController tied to the closing of a ServerResponse (or other
 * appropriate Writable).
 *
 * If the `close` event is fired before the `finish` event, then we'll send the
 * `abort` signal.
 */ export function createAbortController(response) {
    const controller = new AbortController();
    // If `finish` fires first, then `res.end()` has been called and the close is
    // just us finishing the stream on our side. If `close` fires first, then we
    // know the client disconnected before we finished.
    response.once("close", ()=>{
        if (response.writableFinished) return;
        controller.abort(new ResponseAborted());
    });
    return controller;
}
/**
 * Creates an AbortSignal tied to the closing of a ServerResponse (or other
 * appropriate Writable).
 *
 * This cannot be done with the request (IncomingMessage or Readable) because
 * the `abort` event will not fire if to data has been fully read (because that
 * will "close" the readable stream and nothing fires after that).
 */ export function signalFromNodeResponse(response) {
    const { errored, destroyed } = response;
    if (errored || destroyed) {
        return AbortSignal.abort(errored ?? new ResponseAborted());
    }
    const { signal } = createAbortController(response);
    return signal;
}
export class NextRequestAdapter {
    static fromBaseNextRequest(request, signal) {
        // TODO: look at refining this check
        if ("request" in request && request.request) {
            return NextRequestAdapter.fromWebNextRequest(request);
        }
        return NextRequestAdapter.fromNodeNextRequest(request, signal);
    }
    static fromNodeNextRequest(request, signal) {
        // HEAD and GET requests can not have a body.
        let body = null;
        if (request.method !== "GET" && request.method !== "HEAD" && request.body) {
            // @ts-expect-error - this is handled by undici, when streams/web land use it instead
            body = request.body;
        }
        let url;
        if (request.url.startsWith("http")) {
            url = new URL(request.url);
        } else {
            // Grab the full URL from the request metadata.
            const base = getRequestMeta(request, "initURL");
            if (!base || !base.startsWith("http")) {
                // Because the URL construction relies on the fact that the URL provided
                // is absolute, we need to provide a base URL. We can't use the request
                // URL because it's relative, so we use a dummy URL instead.
                url = new URL(request.url, "http://n");
            } else {
                url = new URL(request.url, base);
            }
        }
        return new NextRequest(url, {
            method: request.method,
            headers: fromNodeOutgoingHttpHeaders(request.headers),
            // @ts-expect-error - see https://github.com/whatwg/fetch/pull/1457
            duplex: "half",
            signal,
            // geo
            // ip
            // nextConfig
            // body can not be passed if request was aborted
            // or we get a Request body was disturbed error
            ...signal.aborted ? {} : {
                body
            }
        });
    }
    static fromWebNextRequest(request) {
        // HEAD and GET requests can not have a body.
        let body = null;
        if (request.method !== "GET" && request.method !== "HEAD") {
            body = request.body;
        }
        return new NextRequest(request.url, {
            method: request.method,
            headers: fromNodeOutgoingHttpHeaders(request.headers),
            // @ts-expect-error - see https://github.com/whatwg/fetch/pull/1457
            duplex: "half",
            signal: request.request.signal,
            // geo
            // ip
            // nextConfig
            // body can not be passed if request was aborted
            // or we get a Request body was disturbed error
            ...request.request.signal.aborted ? {} : {
                body
            }
        });
    }
}

//# sourceMappingURL=next-request.js.map