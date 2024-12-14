"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "setHttpClientAndAgentOptions", {
    enumerable: true,
    get: function() {
        return setHttpClientAndAgentOptions;
    }
});
const _http = require("http");
const _https = require("https");
function setHttpClientAndAgentOptions(config) {
    if (globalThis.__NEXT_HTTP_AGENT) {
        // We only need to assign once because we want
        // to reuse the same agent for all requests.
        return;
    }
    if (!config) {
        throw new Error('Expected config.httpAgentOptions to be an object');
    }
    globalThis.__NEXT_HTTP_AGENT_OPTIONS = config.httpAgentOptions;
    globalThis.__NEXT_HTTP_AGENT = new _http.Agent(config.httpAgentOptions);
    globalThis.__NEXT_HTTPS_AGENT = new _https.Agent(config.httpAgentOptions);
}

//# sourceMappingURL=setup-http-agent-env.js.map