"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "walkTreeWithFlightRouterState", {
    enumerable: true,
    get: function() {
        return walkTreeWithFlightRouterState;
    }
});
const _matchsegments = require("../../client/components/match-segments");
const _getcssinlinedlinktags = require("./get-css-inlined-link-tags");
const _getpreloadablefonts = require("./get-preloadable-fonts");
const _createflightrouterstatefromloadertree = require("./create-flight-router-state-from-loader-tree");
const _hasloadingcomponentintree = require("./has-loading-component-in-tree");
const _segment = require("../../shared/lib/segment");
const _createcomponenttree = require("./create-component-tree");
async function walkTreeWithFlightRouterState({ createSegmentPath, loaderTreeToFilter, parentParams, isFirst, flightRouterState, parentRendered, rscPayloadHead, injectedCSS, injectedJS, injectedFontPreloadTags, rootLayoutIncluded, getMetadataReady, ctx, preloadCallbacks }) {
    const { renderOpts: { nextFontManifest, experimental }, query, isPrefetch, getDynamicParamFromSegment } = ctx;
    const [segment, parallelRoutes, modules] = loaderTreeToFilter;
    const parallelRoutesKeys = Object.keys(parallelRoutes);
    const { layout } = modules;
    const isLayout = typeof layout !== 'undefined';
    /**
   * Checks if the current segment is a root layout.
   */ const rootLayoutAtThisLevel = isLayout && !rootLayoutIncluded;
    /**
   * Checks if the current segment or any level above it has a root layout.
   */ const rootLayoutIncludedAtThisLevelOrAbove = rootLayoutIncluded || rootLayoutAtThisLevel;
    // Because this function walks to a deeper point in the tree to start rendering we have to track the dynamic parameters up to the point where rendering starts
    const segmentParam = getDynamicParamFromSegment(segment);
    const currentParams = // Handle null case where dynamic param is optional
    segmentParam && segmentParam.value !== null ? {
        ...parentParams,
        [segmentParam.param]: segmentParam.value
    } : parentParams;
    const actualSegment = (0, _segment.addSearchParamsIfPageSegment)(segmentParam ? segmentParam.treeSegment : segment, query);
    /**
   * Decide if the current segment is where rendering has to start.
   */ const renderComponentsOnThisLevel = // No further router state available
    !flightRouterState || // Segment in router state does not match current segment
    !(0, _matchsegments.matchSegment)(actualSegment, flightRouterState[0]) || // Last item in the tree
    parallelRoutesKeys.length === 0 || // Explicit refresh
    flightRouterState[3] === 'refetch';
    // Pre-PPR, the `loading` component signals to the router how deep to render the component tree
    // to ensure prefetches are quick and inexpensive. If there's no `loading` component anywhere in the tree being rendered,
    // the prefetch will be short-circuited to avoid requesting a potentially very expensive subtree. If there's a `loading`
    // somewhere in the tree, we'll recursively render the component tree up until we encounter that loading component, and then stop.
    const shouldSkipComponentTree = !experimental.isRoutePPREnabled && isPrefetch && !Boolean(modules.loading) && !(0, _hasloadingcomponentintree.hasLoadingComponentInTree)(loaderTreeToFilter);
    if (!parentRendered && renderComponentsOnThisLevel) {
        const overriddenSegment = flightRouterState && (0, _matchsegments.canSegmentBeOverridden)(actualSegment, flightRouterState[0]) ? flightRouterState[0] : actualSegment;
        const routerState = (0, _createflightrouterstatefromloadertree.createFlightRouterStateFromLoaderTree)(// Create router state using the slice of the loaderTree
        loaderTreeToFilter, getDynamicParamFromSegment, query);
        if (shouldSkipComponentTree) {
            // Send only the router state
            return [
                [
                    overriddenSegment,
                    routerState,
                    null,
                    // TODO: It's possible that all the segment data was prefetched during
                    // a navigation, but the head was not. Should we send it down
                    // here anyway?
                    null,
                    false
                ]
            ];
        } else {
            // Create component tree using the slice of the loaderTree
            const seedData = await (0, _createcomponenttree.createComponentTree)(// This ensures flightRouterPath is valid and filters down the tree
            {
                ctx,
                createSegmentPath,
                loaderTree: loaderTreeToFilter,
                parentParams: currentParams,
                firstItem: isFirst,
                injectedCSS,
                injectedJS,
                injectedFontPreloadTags,
                // This is intentionally not "rootLayoutIncludedAtThisLevelOrAbove" as createComponentTree starts at the current level and does a check for "rootLayoutAtThisLevel" too.
                rootLayoutIncluded,
                getMetadataReady,
                preloadCallbacks,
                authInterrupts: experimental.authInterrupts
            });
            return [
                [
                    overriddenSegment,
                    routerState,
                    seedData,
                    rscPayloadHead,
                    false
                ]
            ];
        }
    }
    // If we are not rendering on this level we need to check if the current
    // segment has a layout. If so, we need to track all the used CSS to make
    // the result consistent.
    const layoutPath = layout == null ? void 0 : layout[1];
    const injectedCSSWithCurrentLayout = new Set(injectedCSS);
    const injectedJSWithCurrentLayout = new Set(injectedJS);
    const injectedFontPreloadTagsWithCurrentLayout = new Set(injectedFontPreloadTags);
    if (layoutPath) {
        (0, _getcssinlinedlinktags.getLinkAndScriptTags)(ctx.clientReferenceManifest, layoutPath, injectedCSSWithCurrentLayout, injectedJSWithCurrentLayout, true);
        (0, _getpreloadablefonts.getPreloadableFonts)(nextFontManifest, layoutPath, injectedFontPreloadTagsWithCurrentLayout);
    }
    const paths = [];
    // Walk through all parallel routes.
    for (const parallelRouteKey of parallelRoutesKeys){
        const parallelRoute = parallelRoutes[parallelRouteKey];
        const currentSegmentPath = isFirst ? [
            parallelRouteKey
        ] : [
            actualSegment,
            parallelRouteKey
        ];
        const subPaths = await walkTreeWithFlightRouterState({
            ctx,
            createSegmentPath: (child)=>{
                return createSegmentPath([
                    ...currentSegmentPath,
                    ...child
                ]);
            },
            loaderTreeToFilter: parallelRoute,
            parentParams: currentParams,
            flightRouterState: flightRouterState && flightRouterState[1][parallelRouteKey],
            parentRendered: parentRendered || renderComponentsOnThisLevel,
            isFirst: false,
            rscPayloadHead,
            injectedCSS: injectedCSSWithCurrentLayout,
            injectedJS: injectedJSWithCurrentLayout,
            injectedFontPreloadTags: injectedFontPreloadTagsWithCurrentLayout,
            rootLayoutIncluded: rootLayoutIncludedAtThisLevelOrAbove,
            getMetadataReady,
            preloadCallbacks
        });
        for (const subPath of subPaths){
            // we don't need to send over default routes in the flight data
            // because they are always ignored by the client, unless it's a refetch
            if (subPath[0] === _segment.DEFAULT_SEGMENT_KEY && flightRouterState && !!flightRouterState[1][parallelRouteKey][0] && flightRouterState[1][parallelRouteKey][3] !== 'refetch') {
                continue;
            }
            paths.push([
                actualSegment,
                parallelRouteKey,
                ...subPath
            ]);
        }
    }
    return paths;
}

//# sourceMappingURL=walk-tree-with-flight-router-state.js.map