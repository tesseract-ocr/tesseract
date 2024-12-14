"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "createInitialRouterState", {
    enumerable: true,
    get: function() {
        return createInitialRouterState;
    }
});
const _createhreffromurl = require("./create-href-from-url");
const _filllazyitemstillleafwithhead = require("./fill-lazy-items-till-leaf-with-head");
const _computechangedpath = require("./compute-changed-path");
const _prefetchcacheutils = require("./prefetch-cache-utils");
const _routerreducertypes = require("./router-reducer-types");
const _refetchinactiveparallelsegments = require("./refetch-inactive-parallel-segments");
const _flightdatahelpers = require("../../flight-data-helpers");
function createInitialRouterState(param) {
    let { initialFlightData, initialCanonicalUrlParts, initialParallelRoutes, location, couldBeIntercepted, postponed, prerendered } = param;
    // When initialized on the server, the canonical URL is provided as an array of parts.
    // This is to ensure that when the RSC payload streamed to the client, crawlers don't interpret it
    // as a URL that should be crawled.
    const initialCanonicalUrl = initialCanonicalUrlParts.join('/');
    const normalizedFlightData = (0, _flightdatahelpers.getFlightDataPartsFromPath)(initialFlightData[0]);
    const { tree: initialTree, seedData: initialSeedData, head: initialHead } = normalizedFlightData;
    const isServer = !location;
    // For the SSR render, seed data should always be available (we only send back a `null` response
    // in the case of a `loading` segment, pre-PPR.)
    const rsc = initialSeedData == null ? void 0 : initialSeedData[1];
    var _initialSeedData_;
    const loading = (_initialSeedData_ = initialSeedData == null ? void 0 : initialSeedData[3]) != null ? _initialSeedData_ : null;
    const cache = {
        lazyData: null,
        rsc,
        prefetchRsc: null,
        head: null,
        prefetchHead: null,
        // The cache gets seeded during the first render. `initialParallelRoutes` ensures the cache from the first render is there during the second render.
        parallelRoutes: isServer ? new Map() : initialParallelRoutes,
        loading
    };
    const canonicalUrl = // location.href is read as the initial value for canonicalUrl in the browser
    // This is safe to do as canonicalUrl can't be rendered, it's only used to control the history updates in the useEffect further down in this file.
    location ? (0, _createhreffromurl.createHrefFromUrl)(location) : initialCanonicalUrl;
    (0, _refetchinactiveparallelsegments.addRefreshMarkerToActiveParallelSegments)(initialTree, canonicalUrl);
    const prefetchCache = new Map();
    // When the cache hasn't been seeded yet we fill the cache with the head.
    if (initialParallelRoutes === null || initialParallelRoutes.size === 0) {
        (0, _filllazyitemstillleafwithhead.fillLazyItemsTillLeafWithHead)(cache, undefined, initialTree, initialSeedData, initialHead);
    }
    var // the || operator is intentional, the pathname can be an empty string
    _ref;
    const initialState = {
        tree: initialTree,
        cache,
        prefetchCache,
        pushRef: {
            pendingPush: false,
            mpaNavigation: false,
            // First render needs to preserve the previous window.history.state
            // to avoid it being overwritten on navigation back/forward with MPA Navigation.
            preserveCustomHistoryState: true
        },
        focusAndScrollRef: {
            apply: false,
            onlyHashChange: false,
            hashFragment: null,
            segmentPaths: []
        },
        canonicalUrl,
        nextUrl: (_ref = (0, _computechangedpath.extractPathFromFlightRouterState)(initialTree) || (location == null ? void 0 : location.pathname)) != null ? _ref : null
    };
    if (process.env.NODE_ENV !== 'development' && location) {
        // Seed the prefetch cache with this page's data.
        // This is to prevent needlessly re-prefetching a page that is already reusable,
        // and will avoid triggering a loading state/data fetch stall when navigating back to the page.
        // We don't currently do this in development because links aren't prefetched in development
        // so having a mismatch between prefetch/no prefetch provides inconsistent behavior based on which page
        // was loaded first.
        const url = new URL("" + location.pathname + location.search, location.origin);
        (0, _prefetchcacheutils.createSeededPrefetchCacheEntry)({
            url,
            data: {
                flightData: [
                    normalizedFlightData
                ],
                canonicalUrl: undefined,
                couldBeIntercepted: !!couldBeIntercepted,
                prerendered,
                postponed,
                // TODO: The initial RSC payload includes both static and dynamic data
                // in the same response, even if PPR is enabled. So if there's any
                // dynamic data at all, we can't set a stale time. In the future we may
                // add a way to split a single Flight stream into static and dynamic
                // parts. But in the meantime we should at least make this work for
                // fully static pages.
                staleTime: -1
            },
            tree: initialState.tree,
            prefetchCache: initialState.prefetchCache,
            nextUrl: initialState.nextUrl,
            kind: prerendered ? _routerreducertypes.PrefetchKind.FULL : _routerreducertypes.PrefetchKind.AUTO
        });
    }
    return initialState;
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=create-initial-router-state.js.map