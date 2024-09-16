"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "applyNextWorkerFixture", {
    enumerable: true,
    get: function() {
        return applyNextWorkerFixture;
    }
});
const _proxy = require("../proxy");
class NextWorkerFixtureImpl {
    async setup() {
        const server = await (0, _proxy.createProxyServer)({
            onFetch: this.handleProxyFetch.bind(this)
        });
        this.proxyPort = server.port;
        this.proxyServer = server;
    }
    teardown() {
        if (this.proxyServer) {
            this.proxyServer.close();
            this.proxyServer = null;
        }
    }
    cleanupTest(testId) {
        this.proxyFetchMap.delete(testId);
    }
    onFetch(testId, handler) {
        this.proxyFetchMap.set(testId, handler);
    }
    async handleProxyFetch(testId, request) {
        const handler = this.proxyFetchMap.get(testId);
        return handler == null ? void 0 : handler(request);
    }
    constructor(){
        this.proxyPort = 0;
        this.proxyServer = null;
        this.proxyFetchMap = new Map();
    }
}
async function applyNextWorkerFixture(use) {
    const fixture = new NextWorkerFixtureImpl();
    await fixture.setup();
    await use(fixture);
    fixture.teardown();
}

//# sourceMappingURL=next-worker-fixture.js.map