import { RedirectStatusCode } from "../../client/components/redirect-status-code";
import { getCookieParser } from "../api-utils/get-cookie-parser";
export class BaseNextRequest {
    constructor(method, url, body){
        this.method = method;
        this.url = url;
        this.body = body;
    }
    // Utils implemented using the abstract methods above
    get cookies() {
        if (this._cookies) return this._cookies;
        return this._cookies = getCookieParser(this.headers)();
    }
}
export class BaseNextResponse {
    constructor(destination){
        this.destination = destination;
    }
    // Utils implemented using the abstract methods above
    redirect(destination, statusCode) {
        this.setHeader("Location", destination);
        this.statusCode = statusCode;
        // Since IE11 doesn't support the 308 header add backwards
        // compatibility using refresh header
        if (statusCode === RedirectStatusCode.PermanentRedirect) {
            this.setHeader("Refresh", `0;url=${destination}`);
        }
        return this;
    }
}

//# sourceMappingURL=index.js.map