"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "interceptHttpGet", {
    enumerable: true,
    get: function() {
        return interceptHttpGet;
    }
});
const _ClientRequest = require("next/dist/compiled/@mswjs/interceptors/ClientRequest");
const _fetch = require("./fetch");
function interceptHttpGet(originalFetch) {
    const clientRequestInterceptor = new _ClientRequest.ClientRequestInterceptor();
    clientRequestInterceptor.on("request", async ({ request })=>{
        const response = await (0, _fetch.handleFetch)(originalFetch, request);
        request.respondWith(response);
    });
    clientRequestInterceptor.apply();
    // Cleanup.
    return ()=>{
        clientRequestInterceptor.dispose();
    };
}

//# sourceMappingURL=httpget.js.map