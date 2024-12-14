"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "serverActionReducer", {
    enumerable: true,
    get: function() {
        return serverActionReducer;
    }
});
const _appcallserver = require("../../../app-call-server");
const _appfindsourcemapurl = require("../../../app-find-source-map-url");
const _approuterheaders = require("../../app-router-headers");
const _routerreducertypes = require("../router-reducer-types");
const _assignlocation = require("../../../assign-location");
const _createhreffromurl = require("../create-href-from-url");
const _navigatereducer = require("./navigate-reducer");
const _applyrouterstatepatchtotree = require("../apply-router-state-patch-to-tree");
const _isnavigatingtonewrootlayout = require("../is-navigating-to-new-root-layout");
const _handlemutable = require("../handle-mutable");
const _filllazyitemstillleafwithhead = require("../fill-lazy-items-till-leaf-with-head");
const _approuter = require("../../app-router");
const _hasinterceptionrouteincurrenttree = require("./has-interception-route-in-current-tree");
const _handlesegmentmismatch = require("../handle-segment-mismatch");
const _refetchinactiveparallelsegments = require("../refetch-inactive-parallel-segments");
const _flightdatahelpers = require("../../../flight-data-helpers");
const _redirect = require("../../redirect");
const _redirecterror = require("../../redirect-error");
const _prefetchcacheutils = require("../prefetch-cache-utils");
const _removebasepath = require("../../../remove-base-path");
const _hasbasepath = require("../../../has-base-path");
const _serverreferenceinfo = require("./server-reference-info");
// // eslint-disable-next-line import/no-extraneous-dependencies
// import { createFromFetch } from 'react-server-dom-webpack/client'
// // eslint-disable-next-line import/no-extraneous-dependencies
// import { encodeReply } from 'react-server-dom-webpack/client'
const { createFromFetch, createTemporaryReferenceSet, encodeReply } = !!process.env.NEXT_RUNTIME ? require('react-server-dom-webpack/client.edge') : require('react-server-dom-webpack/client');
async function fetchServerAction(state, nextUrl, param) {
    let { actionId, actionArgs } = param;
    const temporaryReferences = createTemporaryReferenceSet();
    const info = (0, _serverreferenceinfo.extractInfoFromServerReferenceId)(actionId);
    // TODO: Currently, we're only omitting unused args for the experimental "use
    // cache" functions. Once the server reference info byte feature is stable, we
    // should apply this to server actions as well.
    const usedArgs = info.type === 'use-cache' ? (0, _serverreferenceinfo.omitUnusedArgs)(actionArgs, info) : actionArgs;
    const body = await encodeReply(usedArgs, {
        temporaryReferences
    });
    const res = await fetch('', {
        method: 'POST',
        headers: {
            Accept: _approuterheaders.RSC_CONTENT_TYPE_HEADER,
            [_approuterheaders.ACTION_HEADER]: actionId,
            [_approuterheaders.NEXT_ROUTER_STATE_TREE_HEADER]: encodeURIComponent(JSON.stringify(state.tree)),
            ...process.env.NEXT_DEPLOYMENT_ID ? {
                'x-deployment-id': process.env.NEXT_DEPLOYMENT_ID
            } : {},
            ...nextUrl ? {
                [_approuterheaders.NEXT_URL]: nextUrl
            } : {}
        },
        body
    });
    const redirectHeader = res.headers.get('x-action-redirect');
    const [location, _redirectType] = (redirectHeader == null ? void 0 : redirectHeader.split(';')) || [];
    let redirectType;
    switch(_redirectType){
        case 'push':
            redirectType = _redirecterror.RedirectType.push;
            break;
        case 'replace':
            redirectType = _redirecterror.RedirectType.replace;
            break;
        default:
            redirectType = undefined;
    }
    const isPrerender = !!res.headers.get(_approuterheaders.NEXT_IS_PRERENDER_HEADER);
    let revalidatedParts;
    try {
        const revalidatedHeader = JSON.parse(res.headers.get('x-action-revalidated') || '[[],0,0]');
        revalidatedParts = {
            paths: revalidatedHeader[0] || [],
            tag: !!revalidatedHeader[1],
            cookie: revalidatedHeader[2]
        };
    } catch (e) {
        revalidatedParts = {
            paths: [],
            tag: false,
            cookie: false
        };
    }
    const redirectLocation = location ? (0, _assignlocation.assignLocation)(location, new URL(state.canonicalUrl, window.location.href)) : undefined;
    const contentType = res.headers.get('content-type');
    if (contentType == null ? void 0 : contentType.startsWith(_approuterheaders.RSC_CONTENT_TYPE_HEADER)) {
        const response = await createFromFetch(Promise.resolve(res), {
            callServer: _appcallserver.callServer,
            findSourceMapURL: _appfindsourcemapurl.findSourceMapURL,
            temporaryReferences
        });
        if (location) {
            // if it was a redirection, then result is just a regular RSC payload
            return {
                actionFlightData: (0, _flightdatahelpers.normalizeFlightData)(response.f),
                redirectLocation,
                redirectType,
                revalidatedParts,
                isPrerender
            };
        }
        return {
            actionResult: response.a,
            actionFlightData: (0, _flightdatahelpers.normalizeFlightData)(response.f),
            redirectLocation,
            redirectType,
            revalidatedParts,
            isPrerender
        };
    }
    // Handle invalid server action responses
    if (res.status >= 400) {
        // The server can respond with a text/plain error message, but we'll fallback to something generic
        // if there isn't one.
        const error = contentType === 'text/plain' ? await res.text() : 'An unexpected response was received from the server.';
        throw new Error(error);
    }
    return {
        redirectLocation,
        redirectType,
        revalidatedParts,
        isPrerender
    };
}
function serverActionReducer(state, action) {
    const { resolve, reject } = action;
    const mutable = {};
    let currentTree = state.tree;
    mutable.preserveCustomHistoryState = false;
    // only pass along the `nextUrl` param (used for interception routes) if the current route was intercepted.
    // If the route has been intercepted, the action should be as well.
    // Otherwise the server action might be intercepted with the wrong action id
    // (ie, one that corresponds with the intercepted route)
    const nextUrl = state.nextUrl && (0, _hasinterceptionrouteincurrenttree.hasInterceptionRouteInCurrentTree)(state.tree) ? state.nextUrl : null;
    return fetchServerAction(state, nextUrl, action).then(async (param)=>{
        let { actionResult, actionFlightData: flightData, redirectLocation, redirectType, isPrerender, revalidatedParts } = param;
        let redirectHref;
        // honor the redirect type instead of defaulting to push in case of server actions.
        if (redirectLocation) {
            if (redirectType === _redirecterror.RedirectType.replace) {
                state.pushRef.pendingPush = false;
                mutable.pendingPush = false;
            } else {
                state.pushRef.pendingPush = true;
                mutable.pendingPush = true;
            }
            redirectHref = (0, _createhreffromurl.createHrefFromUrl)(redirectLocation, false);
            mutable.canonicalUrl = redirectHref;
        }
        if (!flightData) {
            resolve(actionResult);
            // If there is a redirect but no flight data we need to do a mpaNavigation.
            if (redirectLocation) {
                return (0, _navigatereducer.handleExternalUrl)(state, mutable, redirectLocation.href, state.pushRef.pendingPush);
            }
            return state;
        }
        if (typeof flightData === 'string') {
            // Handle case when navigating to page in `pages` from `app`
            resolve(actionResult);
            return (0, _navigatereducer.handleExternalUrl)(state, mutable, flightData, state.pushRef.pendingPush);
        }
        const actionRevalidated = revalidatedParts.paths.length > 0 || revalidatedParts.tag || revalidatedParts.cookie;
        for (const normalizedFlightData of flightData){
            const { tree: treePatch, seedData: cacheNodeSeedData, head, isRootRender } = normalizedFlightData;
            if (!isRootRender) {
                // TODO-APP: handle this case better
                console.log('SERVER ACTION APPLY FAILED');
                resolve(actionResult);
                return state;
            }
            // Given the path can only have two items the items are only the router state and rsc for the root.
            const newTree = (0, _applyrouterstatepatchtotree.applyRouterStatePatchToTree)(// TODO-APP: remove ''
            [
                ''
            ], currentTree, treePatch, redirectHref ? redirectHref : state.canonicalUrl);
            if (newTree === null) {
                resolve(actionResult);
                return (0, _handlesegmentmismatch.handleSegmentMismatch)(state, action, treePatch);
            }
            if ((0, _isnavigatingtonewrootlayout.isNavigatingToNewRootLayout)(currentTree, newTree)) {
                resolve(actionResult);
                return (0, _navigatereducer.handleExternalUrl)(state, mutable, redirectHref || state.canonicalUrl, state.pushRef.pendingPush);
            }
            // The server sent back RSC data for the server action, so we need to apply it to the cache.
            if (cacheNodeSeedData !== null) {
                const rsc = cacheNodeSeedData[1];
                const cache = (0, _approuter.createEmptyCacheNode)();
                cache.rsc = rsc;
                cache.prefetchRsc = null;
                cache.loading = cacheNodeSeedData[3];
                (0, _filllazyitemstillleafwithhead.fillLazyItemsTillLeafWithHead)(cache, // Existing cache is not passed in as server actions have to invalidate the entire cache.
                undefined, treePatch, cacheNodeSeedData, head);
                mutable.cache = cache;
                mutable.prefetchCache = new Map();
                if (actionRevalidated) {
                    await (0, _refetchinactiveparallelsegments.refreshInactiveParallelSegments)({
                        state,
                        updatedTree: newTree,
                        updatedCache: cache,
                        includeNextUrl: Boolean(nextUrl),
                        canonicalUrl: mutable.canonicalUrl || state.canonicalUrl
                    });
                }
            }
            mutable.patchedTree = newTree;
            currentTree = newTree;
        }
        if (redirectLocation && redirectHref) {
            // Because the RedirectBoundary will trigger a navigation, we need to seed the prefetch cache
            // with the FlightData that we got from the server action for the target page, so that it's
            // available when the page is navigated to and doesn't need to be re-fetched.
            // We only do this if the server action didn't revalidate any data, as in that case the
            // client cache will be cleared and the data will be re-fetched anyway.
            if (!actionRevalidated) {
                (0, _prefetchcacheutils.createSeededPrefetchCacheEntry)({
                    url: redirectLocation,
                    data: {
                        flightData,
                        canonicalUrl: undefined,
                        couldBeIntercepted: false,
                        prerendered: false,
                        postponed: false,
                        // TODO: We should be able to set this if the server action
                        // returned a fully static response.
                        staleTime: -1
                    },
                    tree: state.tree,
                    prefetchCache: state.prefetchCache,
                    nextUrl: state.nextUrl,
                    kind: isPrerender ? _routerreducertypes.PrefetchKind.FULL : _routerreducertypes.PrefetchKind.AUTO
                });
                mutable.prefetchCache = state.prefetchCache;
            }
            // If the action triggered a redirect, the action promise promise will be rejected with
            // a redirect so that it's handled by RedirectBoundary as we won't have a valid
            // action result to resolve the promise with. This will effectively reset the state of
            // the component that called the action as the error boundary will remount the tree.
            // The status code doesn't matter here as the action handler will have already sent
            // a response with the correct status code.
            reject((0, _redirect.getRedirectError)((0, _hasbasepath.hasBasePath)(redirectHref) ? (0, _removebasepath.removeBasePath)(redirectHref) : redirectHref, redirectType || _redirecterror.RedirectType.push));
        } else {
            resolve(actionResult);
        }
        return (0, _handlemutable.handleMutable)(state, mutable);
    }, (e)=>{
        // When the server action is rejected we don't update the state and instead call the reject handler of the promise.
        reject(e);
        return state;
    });
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=server-action-reducer.js.map