import { toNodeOutgoingHttpHeaders } from "../web/utils";
import { BaseNextRequest, BaseNextResponse } from "./index";
import { DetachedPromise } from "../../lib/detached-promise";
export class WebNextRequest extends BaseNextRequest {
    constructor(request){
        const url = new URL(request.url);
        super(request.method, url.href.slice(url.origin.length), request.clone().body);
        this.request = request;
        this.fetchMetrics = request.fetchMetrics;
        this.headers = {};
        for (const [name, value] of request.headers.entries()){
            this.headers[name] = value;
        }
    }
    async parseBody(_limit) {
        throw new Error("parseBody is not implemented in the web runtime");
    }
}
export class WebNextResponse extends BaseNextResponse {
    constructor(transformStream = new TransformStream()){
        super(transformStream.writable);
        this.transformStream = transformStream;
        this.headers = new Headers();
        this.textBody = undefined;
        this.sendPromise = new DetachedPromise();
        this._sent = false;
    }
    setHeader(name, value) {
        this.headers.delete(name);
        for (const val of Array.isArray(value) ? value : [
            value
        ]){
            this.headers.append(name, val);
        }
        return this;
    }
    removeHeader(name) {
        this.headers.delete(name);
        return this;
    }
    getHeaderValues(name) {
        var _this_getHeader;
        // https://developer.mozilla.org/docs/Web/API/Headers/get#example
        return (_this_getHeader = this.getHeader(name)) == null ? void 0 : _this_getHeader.split(",").map((v)=>v.trimStart());
    }
    getHeader(name) {
        return this.headers.get(name) ?? undefined;
    }
    getHeaders() {
        return toNodeOutgoingHttpHeaders(this.headers);
    }
    hasHeader(name) {
        return this.headers.has(name);
    }
    appendHeader(name, value) {
        this.headers.append(name, value);
        return this;
    }
    body(value) {
        this.textBody = value;
        return this;
    }
    send() {
        this.sendPromise.resolve();
        this._sent = true;
    }
    get sent() {
        return this._sent;
    }
    async toResponse() {
        // If we haven't called `send` yet, wait for it to be called.
        if (!this.sent) await this.sendPromise.promise;
        return new Response(this.textBody ?? this.transformStream.readable, {
            headers: this.headers,
            status: this.statusCode,
            statusText: this.statusMessage
        });
    }
}

//# sourceMappingURL=web.js.map