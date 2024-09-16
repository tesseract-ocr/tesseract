"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    createPrefetchCacheEntryForInitialLoad: null,
    getOrCreatePrefetchCacheEntry: null,
    prunePrefetchCache: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    createPrefetchCacheEntryForInitialLoad: function() {
        return createPrefetchCacheEntryForInitialLoad;
    },
    getOrCreatePrefetchCacheEntry: function() {
        return getOrCreatePrefetchCacheEntry;
    },
    prunePrefetchCache: function() {
        return prunePrefetchCache;
    }
});
const _createhreffromurl = require("./create-href-from-url");
const _fetchserverresponse = require("./fetch-server-response");
const _routerreducertypes = require("./router-reducer-types");
const _prefetchreducer = require("./reducers/prefetch-reducer");
/**
 * Creates a cache key for the router prefetch cache
 *
 * @param url - The URL being navigated to
 * @param nextUrl - an internal URL, primarily used for handling rewrites. Defaults to '/'.
 * @return The generated prefetch cache key.
 */ function createPrefetchCacheKey(url, nextUrl) {
    const pathnameFromUrl = (0, _createhreffromurl.createHrefFromUrl)(url, // Ensures the hash is not part of the cache key as it does not impact the server fetch
    false);
    // nextUrl is used as a cache key delimiter since entries can vary based on the Next-URL header
    if (nextUrl) {
        return nextUrl + "%" + pathnameFromUrl;
    }
    return pathnameFromUrl;
}
function getOrCreatePrefetchCacheEntry(param) {
    let { url, nextUrl, tree, buildId, prefetchCache, kind } = param;
    let existingCacheEntry = undefined;
    // We first check if there's a more specific interception route prefetch entry
    // This is because when we detect a prefetch that corresponds with an interception route, we prefix it with nextUrl (see `createPrefetchCacheKey`)
    // to avoid conflicts with other pages that may have the same URL but render different things depending on the `Next-URL` header.
    const interceptionCacheKey = createPrefetchCacheKey(url, nextUrl);
    const interceptionData = prefetchCache.get(interceptionCacheKey);
    if (interceptionData) {
        existingCacheEntry = interceptionData;
    } else {
        // If we dont find a more specific interception route prefetch entry, we check for a regular prefetch entry
        const prefetchCacheKey = createPrefetchCacheKey(url);
        const prefetchData = prefetchCache.get(prefetchCacheKey);
        if (prefetchData) {
            existingCacheEntry = prefetchData;
        }
    }
    if (existingCacheEntry) {
        // Grab the latest status of the cache entry and update it
        existingCacheEntry.status = getPrefetchEntryCacheStatus(existingCacheEntry);
        // when `kind` is provided, an explicit prefetch was requested.
        // if the requested prefetch is "full" and the current cache entry wasn't, we want to re-prefetch with the new intent
        const switchedToFullPrefetch = existingCacheEntry.kind !== _routerreducertypes.PrefetchKind.FULL && kind === _routerreducertypes.PrefetchKind.FULL;
        if (switchedToFullPrefetch) {
            return createLazyPrefetchEntry({
                tree,
                url,
                buildId,
                nextUrl,
                prefetchCache,
                // If we didn't get an explicit prefetch kind, we want to set a temporary kind
                // rather than assuming the same intent as the previous entry, to be consistent with how we
                // lazily create prefetch entries when intent is left unspecified.
                kind: kind != null ? kind : _routerreducertypes.PrefetchKind.TEMPORARY
            });
        }
        // If the existing cache entry was marked as temporary, it means it was lazily created when attempting to get an entry,
        // where we didn't have the prefetch intent. Now that we have the intent (in `kind`), we want to update the entry to the more accurate kind.
        if (kind && existingCacheEntry.kind === _routerreducertypes.PrefetchKind.TEMPORARY) {
            existingCacheEntry.kind = kind;
        }
        // We've determined that the existing entry we found is still valid, so we return it.
        return existingCacheEntry;
    }
    // If we didn't return an entry, create a new one.
    return createLazyPrefetchEntry({
        tree,
        url,
        buildId,
        nextUrl,
        prefetchCache,
        kind: kind || // in dev, there's never gonna be a prefetch entry so we want to prefetch here
        (process.env.NODE_ENV === "development" ? _routerreducertypes.PrefetchKind.AUTO : _routerreducertypes.PrefetchKind.TEMPORARY)
    });
}
/*
 * Used to take an existing cache entry and prefix it with the nextUrl, if it exists.
 * This ensures that we don't have conflicting cache entries for the same URL (as is the case with route interception).
 */ function prefixExistingPrefetchCacheEntry(param) {
    let { url, nextUrl, prefetchCache } = param;
    const existingCacheKey = createPrefetchCacheKey(url);
    const existingCacheEntry = prefetchCache.get(existingCacheKey);
    if (!existingCacheEntry) {
        // no-op -- there wasn't an entry to move
        return;
    }
    const newCacheKey = createPrefetchCacheKey(url, nextUrl);
    prefetchCache.set(newCacheKey, existingCacheEntry);
    prefetchCache.delete(existingCacheKey);
}
function createPrefetchCacheEntryForInitialLoad(param) {
    let { nextUrl, tree, prefetchCache, url, kind, data } = param;
    const [, , , intercept] = data;
    // if the prefetch corresponds with an interception route, we use the nextUrl to prefix the cache key
    const prefetchCacheKey = intercept ? createPrefetchCacheKey(url, nextUrl) : createPrefetchCacheKey(url);
    const prefetchEntry = {
        treeAtTimeOfPrefetch: tree,
        data: Promise.resolve(data),
        kind,
        prefetchTime: Date.now(),
        lastUsedTime: Date.now(),
        key: prefetchCacheKey,
        status: _routerreducertypes.PrefetchCacheEntryStatus.fresh
    };
    prefetchCache.set(prefetchCacheKey, prefetchEntry);
    return prefetchEntry;
}
/**
 * Creates a prefetch entry entry and enqueues a fetch request to retrieve the data.
 */ function createLazyPrefetchEntry(param) {
    let { url, kind, tree, nextUrl, buildId, prefetchCache } = param;
    const prefetchCacheKey = createPrefetchCacheKey(url);
    // initiates the fetch request for the prefetch and attaches a listener
    // to the promise to update the prefetch cache entry when the promise resolves (if necessary)
    const data = _prefetchreducer.prefetchQueue.enqueue(()=>(0, _fetchserverresponse.fetchServerResponse)(url, tree, nextUrl, buildId, kind).then((prefetchResponse)=>{
            // TODO: `fetchServerResponse` should be more tighly coupled to these prefetch cache operations
            // to avoid drift between this cache key prefixing logic
            // (which is currently directly influenced by the server response)
            const [, , , intercepted] = prefetchResponse;
            if (intercepted) {
                prefixExistingPrefetchCacheEntry({
                    url,
                    nextUrl,
                    prefetchCache
                });
            }
            return prefetchResponse;
        }));
    const prefetchEntry = {
        treeAtTimeOfPrefetch: tree,
        data,
        kind,
        prefetchTime: Date.now(),
        lastUsedTime: null,
        key: prefetchCacheKey,
        status: _routerreducertypes.PrefetchCacheEntryStatus.fresh
    };
    prefetchCache.set(prefetchCacheKey, prefetchEntry);
    return prefetchEntry;
}
function prunePrefetchCache(prefetchCache) {
    for (const [href, prefetchCacheEntry] of prefetchCache){
        if (getPrefetchEntryCacheStatus(prefetchCacheEntry) === _routerreducertypes.PrefetchCacheEntryStatus.expired) {
            prefetchCache.delete(href);
        }
    }
}
// These values are set by `define-env-plugin` (based on `nextConfig.experimental.staleTimes`)
// and default to 5 minutes (static) / 30 seconds (dynamic)
const DYNAMIC_STALETIME_MS = Number(process.env.__NEXT_CLIENT_ROUTER_DYNAMIC_STALETIME) * 1000;
const STATIC_STALETIME_MS = Number(process.env.__NEXT_CLIENT_ROUTER_STATIC_STALETIME) * 1000;
function getPrefetchEntryCacheStatus(param) {
    let { kind, prefetchTime, lastUsedTime } = param;
    // We will re-use the cache entry data for up to the `dynamic` staletime window.
    if (Date.now() < (lastUsedTime != null ? lastUsedTime : prefetchTime) + DYNAMIC_STALETIME_MS) {
        return lastUsedTime ? _routerreducertypes.PrefetchCacheEntryStatus.reusable : _routerreducertypes.PrefetchCacheEntryStatus.fresh;
    }
    // For "auto" prefetching, we'll re-use only the loading boundary for up to `static` staletime window.
    // A stale entry will only re-use the `loading` boundary, not the full data.
    // This will trigger a "lazy fetch" for the full data.
    if (kind === "auto") {
        if (Date.now() < prefetchTime + STATIC_STALETIME_MS) {
            return _routerreducertypes.PrefetchCacheEntryStatus.stale;
        }
    }
    // for "full" prefetching, we'll re-use the cache entry data for up to `static` staletime window.
    if (kind === "full") {
        if (Date.now() < prefetchTime + STATIC_STALETIME_MS) {
            return _routerreducertypes.PrefetchCacheEntryStatus.reusable;
        }
    }
    return _routerreducertypes.PrefetchCacheEntryStatus.expired;
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=prefetch-cache-utils.js.map