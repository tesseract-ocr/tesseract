"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    fillCacheWithNewSubTreeData: null,
    fillCacheWithNewSubTreeDataButOnlyLoading: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    fillCacheWithNewSubTreeData: function() {
        return fillCacheWithNewSubTreeData;
    },
    fillCacheWithNewSubTreeDataButOnlyLoading: function() {
        return fillCacheWithNewSubTreeDataButOnlyLoading;
    }
});
const _invalidatecachebyrouterstate = require("./invalidate-cache-by-router-state");
const _filllazyitemstillleafwithhead = require("./fill-lazy-items-till-leaf-with-head");
const _createroutercachekey = require("./create-router-cache-key");
const _segment = require("../../../shared/lib/segment");
/**
 * Common logic for filling cache with new sub tree data.
 */ function fillCacheHelper(newCache, existingCache, flightData, prefetchEntry, fillLazyItems) {
    const { segmentPath, seedData: cacheNodeSeedData, tree: treePatch, head } = flightData;
    let newCacheNode = newCache;
    let existingCacheNode = existingCache;
    for(let i = 0; i < segmentPath.length; i += 2){
        const parallelRouteKey = segmentPath[i];
        const segment = segmentPath[i + 1];
        // segmentPath is a repeating tuple of parallelRouteKey and segment
        // we know we've hit the last entry we've reached our final pair
        const isLastEntry = i === segmentPath.length - 2;
        const cacheKey = (0, _createroutercachekey.createRouterCacheKey)(segment);
        const existingChildSegmentMap = existingCacheNode.parallelRoutes.get(parallelRouteKey);
        if (!existingChildSegmentMap) {
            continue;
        }
        let childSegmentMap = newCacheNode.parallelRoutes.get(parallelRouteKey);
        if (!childSegmentMap || childSegmentMap === existingChildSegmentMap) {
            childSegmentMap = new Map(existingChildSegmentMap);
            newCacheNode.parallelRoutes.set(parallelRouteKey, childSegmentMap);
        }
        const existingChildCacheNode = existingChildSegmentMap.get(cacheKey);
        let childCacheNode = childSegmentMap.get(cacheKey);
        if (isLastEntry) {
            if (cacheNodeSeedData && (!childCacheNode || !childCacheNode.lazyData || childCacheNode === existingChildCacheNode)) {
                const incomingSegment = cacheNodeSeedData[0];
                const rsc = cacheNodeSeedData[1];
                const loading = cacheNodeSeedData[3];
                childCacheNode = {
                    lazyData: null,
                    // When `fillLazyItems` is false, we only want to fill the RSC data for the layout,
                    // not the page segment.
                    rsc: fillLazyItems || incomingSegment !== _segment.PAGE_SEGMENT_KEY ? rsc : null,
                    prefetchRsc: null,
                    head: null,
                    prefetchHead: null,
                    loading,
                    parallelRoutes: fillLazyItems && existingChildCacheNode ? new Map(existingChildCacheNode.parallelRoutes) : new Map()
                };
                if (existingChildCacheNode && fillLazyItems) {
                    (0, _invalidatecachebyrouterstate.invalidateCacheByRouterState)(childCacheNode, existingChildCacheNode, treePatch);
                }
                if (fillLazyItems) {
                    (0, _filllazyitemstillleafwithhead.fillLazyItemsTillLeafWithHead)(childCacheNode, existingChildCacheNode, treePatch, cacheNodeSeedData, head, prefetchEntry);
                }
                childSegmentMap.set(cacheKey, childCacheNode);
            }
            continue;
        }
        if (!childCacheNode || !existingChildCacheNode) {
            continue;
        }
        if (childCacheNode === existingChildCacheNode) {
            childCacheNode = {
                lazyData: childCacheNode.lazyData,
                rsc: childCacheNode.rsc,
                prefetchRsc: childCacheNode.prefetchRsc,
                head: childCacheNode.head,
                prefetchHead: childCacheNode.prefetchHead,
                parallelRoutes: new Map(childCacheNode.parallelRoutes),
                loading: childCacheNode.loading
            };
            childSegmentMap.set(cacheKey, childCacheNode);
        }
        // Move deeper into the cache nodes
        newCacheNode = childCacheNode;
        existingCacheNode = existingChildCacheNode;
    }
}
function fillCacheWithNewSubTreeData(newCache, existingCache, flightData, prefetchEntry) {
    fillCacheHelper(newCache, existingCache, flightData, prefetchEntry, true);
}
function fillCacheWithNewSubTreeDataButOnlyLoading(newCache, existingCache, flightData, prefetchEntry) {
    fillCacheHelper(newCache, existingCache, flightData, prefetchEntry, false);
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=fill-cache-with-new-subtree-data.js.map