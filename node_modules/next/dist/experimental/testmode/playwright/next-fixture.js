"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "applyNextFixture", {
    enumerable: true,
    get: function() {
        return applyNextFixture;
    }
});
const _pageroute = require("./page-route");
const _report = require("./report");
class NextFixtureImpl {
    constructor(testInfo, options, worker, page){
        this.testInfo = testInfo;
        this.options = options;
        this.worker = worker;
        this.page = page;
        this.fetchHandlers = [];
        this.testId = testInfo.testId;
        const testHeaders = {
            "Next-Test-Proxy-Port": String(worker.proxyPort),
            "Next-Test-Data": this.testId
        };
        const handleFetch = this.handleFetch.bind(this);
        worker.onFetch(this.testId, handleFetch);
        this.page.context().route("**", (route)=>(0, _pageroute.handleRoute)(route, page, testHeaders, handleFetch));
    }
    teardown() {
        this.worker.cleanupTest(this.testId);
    }
    onFetch(handler) {
        this.fetchHandlers.push(handler);
    }
    async handleFetch(request) {
        return (0, _report.reportFetch)(this.testInfo, request, async (req)=>{
            for (const handler of this.fetchHandlers.slice().reverse()){
                const result = await handler(req.clone());
                if (result) {
                    return result;
                }
            }
            if (this.options.fetchLoopback) {
                return fetch(req.clone());
            }
            return undefined;
        });
    }
}
async function applyNextFixture(use, { testInfo, nextOptions, nextWorker, page }) {
    const fixture = new NextFixtureImpl(testInfo, nextOptions, nextWorker, page);
    await use(fixture);
    fixture.teardown();
}

//# sourceMappingURL=next-fixture.js.map