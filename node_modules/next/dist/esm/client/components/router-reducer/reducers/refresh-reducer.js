import { fetchServerResponse } from '../fetch-server-response';
import { createHrefFromUrl } from '../create-href-from-url';
import { applyRouterStatePatchToTree } from '../apply-router-state-patch-to-tree';
import { isNavigatingToNewRootLayout } from '../is-navigating-to-new-root-layout';
import { handleExternalUrl } from './navigate-reducer';
import { handleMutable } from '../handle-mutable';
import { fillLazyItemsTillLeafWithHead } from '../fill-lazy-items-till-leaf-with-head';
import { createEmptyCacheNode } from '../../app-router';
import { handleSegmentMismatch } from '../handle-segment-mismatch';
import { hasInterceptionRouteInCurrentTree } from './has-interception-route-in-current-tree';
import { refreshInactiveParallelSegments } from '../refetch-inactive-parallel-segments';
export function refreshReducer(state, action) {
    const { origin } = action;
    const mutable = {};
    const href = state.canonicalUrl;
    let currentTree = state.tree;
    mutable.preserveCustomHistoryState = false;
    const cache = createEmptyCacheNode();
    // If the current tree was intercepted, the nextUrl should be included in the request.
    // This is to ensure that the refresh request doesn't get intercepted, accidentally triggering the interception route.
    const includeNextUrl = hasInterceptionRouteInCurrentTree(state.tree);
    // TODO-APP: verify that `href` is not an external url.
    // Fetch data from the root of the tree.
    cache.lazyData = fetchServerResponse(new URL(href, origin), {
        flightRouterState: [
            currentTree[0],
            currentTree[1],
            currentTree[2],
            'refetch'
        ],
        nextUrl: includeNextUrl ? state.nextUrl : null
    });
    return cache.lazyData.then(async (param)=>{
        let { flightData, canonicalUrl: canonicalUrlOverride } = param;
        // Handle case when navigating to page in `pages` from `app`
        if (typeof flightData === 'string') {
            return handleExternalUrl(state, mutable, flightData, state.pushRef.pendingPush);
        }
        // Remove cache.lazyData as it has been resolved at this point.
        cache.lazyData = null;
        for (const normalizedFlightData of flightData){
            const { tree: treePatch, seedData: cacheNodeSeedData, head, isRootRender } = normalizedFlightData;
            if (!isRootRender) {
                // TODO-APP: handle this case better
                console.log('REFRESH FAILED');
                return state;
            }
            const newTree = applyRouterStatePatchToTree(// TODO-APP: remove ''
            [
                ''
            ], currentTree, treePatch, state.canonicalUrl);
            if (newTree === null) {
                return handleSegmentMismatch(state, action, treePatch);
            }
            if (isNavigatingToNewRootLayout(currentTree, newTree)) {
                return handleExternalUrl(state, mutable, href, state.pushRef.pendingPush);
            }
            const canonicalUrlOverrideHref = canonicalUrlOverride ? createHrefFromUrl(canonicalUrlOverride) : undefined;
            if (canonicalUrlOverride) {
                mutable.canonicalUrl = canonicalUrlOverrideHref;
            }
            // Handles case where prefetch only returns the router tree patch without rendered components.
            if (cacheNodeSeedData !== null) {
                const rsc = cacheNodeSeedData[1];
                const loading = cacheNodeSeedData[3];
                cache.rsc = rsc;
                cache.prefetchRsc = null;
                cache.loading = loading;
                fillLazyItemsTillLeafWithHead(cache, // Existing cache is not passed in as `router.refresh()` has to invalidate the entire cache.
                undefined, treePatch, cacheNodeSeedData, head);
                mutable.prefetchCache = new Map();
            }
            await refreshInactiveParallelSegments({
                state,
                updatedTree: newTree,
                updatedCache: cache,
                includeNextUrl,
                canonicalUrl: mutable.canonicalUrl || state.canonicalUrl
            });
            mutable.cache = cache;
            mutable.patchedTree = newTree;
            currentTree = newTree;
        }
        return handleMutable(state, mutable);
    }, ()=>state);
}

//# sourceMappingURL=refresh-reducer.js.map