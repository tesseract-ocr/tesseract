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
        worker.onFetch(this.testId, this.handleFetch.bind(this));
    }
    async setup() {
        const testHeaders = {
            'Next-Test-Proxy-Port': String(this.worker.proxyPort),
            'Next-Test-Data': this.testId
        };
        await this.page.context().route('**', (route)=>(0, _pageroute.handleRoute)(route, this.page, testHeaders, this.handleFetch.bind(this)));
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
    await fixture.setup();
    // eslint-disable-next-line react-hooks/rules-of-hooks -- not React.use()
    await use(fixture);
    fixture.teardown();
}

//# sourceMappingURL=next-fixture.js.map