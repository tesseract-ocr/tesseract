"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "fastRefreshReducer", {
    enumerable: true,
    get: function() {
        return fastRefreshReducer;
    }
});
const _fetchserverresponse = require("../fetch-server-response");
const _createhreffromurl = require("../create-href-from-url");
const _applyrouterstatepatchtotree = require("../apply-router-state-patch-to-tree");
const _isnavigatingtonewrootlayout = require("../is-navigating-to-new-root-layout");
const _navigatereducer = require("./navigate-reducer");
const _handlemutable = require("../handle-mutable");
const _applyflightdata = require("../apply-flight-data");
const _approuter = require("../../app-router");
const _handlesegmentmismatch = require("../handle-segment-mismatch");
const _hasinterceptionrouteincurrenttree = require("./has-interception-route-in-current-tree");
// A version of refresh reducer that keeps the cache around instead of wiping all of it.
function fastRefreshReducerImpl(state, action) {
    const { origin } = action;
    const mutable = {};
    const href = state.canonicalUrl;
    mutable.preserveCustomHistoryState = false;
    const cache = (0, _approuter.createEmptyCacheNode)();
    // If the current tree was intercepted, the nextUrl should be included in the request.
    // This is to ensure that the refresh request doesn't get intercepted, accidentally triggering the interception route.
    const includeNextUrl = (0, _hasinterceptionrouteincurrenttree.hasInterceptionRouteInCurrentTree)(state.tree);
    // TODO-APP: verify that `href` is not an external url.
    // Fetch data from the root of the tree.
    cache.lazyData = (0, _fetchserverresponse.fetchServerResponse)(new URL(href, origin), [
        state.tree[0],
        state.tree[1],
        state.tree[2],
        "refetch"
    ], includeNextUrl ? state.nextUrl : null, state.buildId);
    return cache.lazyData.then((param)=>{
        let [flightData, canonicalUrlOverride] = param;
        // Handle case when navigating to page in `pages` from `app`
        if (typeof flightData === "string") {
            return (0, _navigatereducer.handleExternalUrl)(state, mutable, flightData, state.pushRef.pendingPush);
        }
        // Remove cache.lazyData as it has been resolved at this point.
        cache.lazyData = null;
        let currentTree = state.tree;
        let currentCache = state.cache;
        for (const flightDataPath of flightData){
            // FlightDataPath with more than two items means unexpected Flight data was returned
            if (flightDataPath.length !== 3) {
                // TODO-APP: handle this case better
                console.log("REFRESH FAILED");
                return state;
            }
            // Given the path can only have two items the items are only the router state and rsc for the root.
            const [treePatch] = flightDataPath;
            const newTree = (0, _applyrouterstatepatchtotree.applyRouterStatePatchToTree)(// TODO-APP: remove ''
            [
                ""
            ], currentTree, treePatch, state.canonicalUrl);
            if (newTree === null) {
                return (0, _handlesegmentmismatch.handleSegmentMismatch)(state, action, treePatch);
            }
            if ((0, _isnavigatingtonewrootlayout.isNavigatingToNewRootLayout)(currentTree, newTree)) {
                return (0, _navigatereducer.handleExternalUrl)(state, mutable, href, state.pushRef.pendingPush);
            }
            const canonicalUrlOverrideHref = canonicalUrlOverride ? (0, _createhreffromurl.createHrefFromUrl)(canonicalUrlOverride) : undefined;
            if (canonicalUrlOverride) {
                mutable.canonicalUrl = canonicalUrlOverrideHref;
            }
            const applied = (0, _applyflightdata.applyFlightData)(currentCache, cache, flightDataPath);
            if (applied) {
                mutable.cache = cache;
                currentCache = cache;
            }
            mutable.patchedTree = newTree;
            mutable.canonicalUrl = href;
            currentTree = newTree;
        }
        return (0, _handlemutable.handleMutable)(state, mutable);
    }, ()=>state);
}
function fastRefreshReducerNoop(state, _action) {
    return state;
}
const fastRefreshReducer = process.env.NODE_ENV === "production" ? fastRefreshReducerNoop : fastRefreshReducerImpl;

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=fast-refresh-reducer.js.map