import { SYMBOL_CLEARED_COOKIES } from "../api-utils";
import { NEXT_REQUEST_META } from "../request-meta";
import { BaseNextRequest, BaseNextResponse } from "./index";
let _NEXT_REQUEST_META = NEXT_REQUEST_META;
export class NodeNextRequest extends BaseNextRequest {
    get originalRequest() {
        // Need to mimic these changes to the original req object for places where we use it:
        // render.tsx, api/ssg requests
        this._req[NEXT_REQUEST_META] = this[NEXT_REQUEST_META];
        this._req.url = this.url;
        this._req.cookies = this.cookies;
        return this._req;
    }
    set originalRequest(value) {
        this._req = value;
    }
    constructor(_req){
        var _this__req;
        super(_req.method.toUpperCase(), _req.url, _req);
        this._req = _req;
        this.headers = this._req.headers;
        this.fetchMetrics = (_this__req = this._req) == null ? void 0 : _this__req.fetchMetrics;
        this[_NEXT_REQUEST_META] = this._req[NEXT_REQUEST_META] || {};
    }
}
export class NodeNextResponse extends BaseNextResponse {
    get originalResponse() {
        if (SYMBOL_CLEARED_COOKIES in this) {
            this._res[SYMBOL_CLEARED_COOKIES] = this[SYMBOL_CLEARED_COOKIES];
        }
        return this._res;
    }
    constructor(_res){
        super(_res);
        this._res = _res;
        this.textBody = undefined;
    }
    get sent() {
        return this._res.finished || this._res.headersSent;
    }
    get statusCode() {
        return this._res.statusCode;
    }
    set statusCode(value) {
        this._res.statusCode = value;
    }
    get statusMessage() {
        return this._res.statusMessage;
    }
    set statusMessage(value) {
        this._res.statusMessage = value;
    }
    setHeader(name, value) {
        this._res.setHeader(name, value);
        return this;
    }
    removeHeader(name) {
        this._res.removeHeader(name);
        return this;
    }
    getHeaderValues(name) {
        const values = this._res.getHeader(name);
        if (values === undefined) return undefined;
        return (Array.isArray(values) ? values : [
            values
        ]).map((value)=>value.toString());
    }
    hasHeader(name) {
        return this._res.hasHeader(name);
    }
    getHeader(name) {
        const values = this.getHeaderValues(name);
        return Array.isArray(values) ? values.join(",") : undefined;
    }
    getHeaders() {
        return this._res.getHeaders();
    }
    appendHeader(name, value) {
        const currentValues = this.getHeaderValues(name) ?? [];
        if (!currentValues.includes(value)) {
            this._res.setHeader(name, [
                ...currentValues,
                value
            ]);
        }
        return this;
    }
    body(value) {
        this.textBody = value;
        return this;
    }
    send() {
        this._res.end(this.textBody);
    }
}

//# sourceMappingURL=node.js.map