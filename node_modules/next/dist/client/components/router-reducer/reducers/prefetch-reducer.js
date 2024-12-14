"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    prefetchQueue: null,
    prefetchReducer: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    prefetchQueue: function() {
        return prefetchQueue;
    },
    prefetchReducer: function() {
        return prefetchReducer;
    }
});
const _promisequeue = require("../../promise-queue");
const _prefetchcacheutils = require("../prefetch-cache-utils");
const prefetchQueue = new _promisequeue.PromiseQueue(5);
const prefetchReducer = process.env.__NEXT_PPR && process.env.__NEXT_CLIENT_SEGMENT_CACHE ? identityReducerWhenSegmentCacheIsEnabled : prefetchReducerImpl;
function identityReducerWhenSegmentCacheIsEnabled(state) {
    // Unlike the old implementation, the Segment Cache doesn't store its data in
    // the router reducer state.
    //
    // This shouldn't be reachable because we wrap the prefetch API in a check,
    // too, which prevents the action from being dispatched. But it's here for
    // clarity + code elimination.
    return state;
}
function prefetchReducerImpl(state, action) {
    // let's prune the prefetch cache before we do anything else
    (0, _prefetchcacheutils.prunePrefetchCache)(state.prefetchCache);
    const { url } = action;
    (0, _prefetchcacheutils.getOrCreatePrefetchCacheEntry)({
        url,
        nextUrl: state.nextUrl,
        prefetchCache: state.prefetchCache,
        kind: action.kind,
        tree: state.tree,
        allowAliasing: true
    });
    return state;
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=prefetch-reducer.js.map