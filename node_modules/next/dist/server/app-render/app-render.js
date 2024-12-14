"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    renderToHTMLOrFlight: null,
    warmFlightResponse: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    renderToHTMLOrFlight: function() {
        return renderToHTMLOrFlight;
    },
    warmFlightResponse: function() {
        return warmFlightResponse;
    }
});
const _jsxruntime = require("react/jsx-runtime");
const _workasyncstorageexternal = require("../app-render/work-async-storage.external");
const _react = /*#__PURE__*/ _interop_require_default(require("react"));
const _renderresult = /*#__PURE__*/ _interop_require_default(require("../render-result"));
const _nodewebstreamshelper = require("../stream-utils/node-web-streams-helper");
const _internalutils = require("../internal-utils");
const _approuterheaders = require("../../client/components/app-router-headers");
const _metadatacontext = require("../../lib/metadata/metadata-context");
const _requeststore = require("../async-storage/request-store");
const _workstore = require("../async-storage/work-store");
const _httpaccessfallback = require("../../client/components/http-access-fallback/http-access-fallback");
const _redirect = require("../../client/components/redirect");
const _redirecterror = require("../../client/components/redirect-error");
const _implicittags = require("../lib/implicit-tags");
const _constants = require("../lib/trace/constants");
const _tracer = require("../lib/trace/tracer");
const _flightrenderresult = require("./flight-render-result");
const _createerrorhandler = require("./create-error-handler");
const _getshortdynamicparamtype = require("./get-short-dynamic-param-type");
const _getsegmentparam = require("./get-segment-param");
const _getscriptnoncefromheader = require("./get-script-nonce-from-header");
const _parseandvalidateflightrouterstate = require("./parse-and-validate-flight-router-state");
const _createflightrouterstatefromloadertree = require("./create-flight-router-state-from-loader-tree");
const _actionhandler = require("./action-handler");
const _bailouttocsr = require("../../shared/lib/lazy-dynamic/bailout-to-csr");
const _log = require("../../build/output/log");
const _requestcookies = require("../web/spec-extension/adapters/request-cookies");
const _serverinsertedhtml = require("./server-inserted-html");
const _requiredscripts = require("./required-scripts");
const _addpathprefix = require("../../shared/lib/router/utils/add-path-prefix");
const _makegetserverinsertedhtml = require("./make-get-server-inserted-html");
const _walktreewithflightrouterstate = require("./walk-tree-with-flight-router-state");
const _createcomponenttree = require("./create-component-tree");
const _getassetquerystring = require("./get-asset-query-string");
const _encryptionutils = require("./encryption-utils");
const _postponedstate = require("./postponed-state");
const _hooksservercontext = require("../../client/components/hooks-server-context");
const _useflightresponse = require("./use-flight-response");
const _staticgenerationbailout = require("../../client/components/static-generation-bailout");
const _formatservererror = require("../../lib/format-server-error");
const _dynamicrendering = require("./dynamic-rendering");
const _clientcomponentrendererlogger = require("../client-component-renderer-logger");
const _actionutils = require("./action-utils");
const _helpers = require("../base-http/helpers");
const _routeregex = require("../../shared/lib/router/utils/route-regex");
const _parserelativeurl = require("../../shared/lib/router/utils/parse-relative-url");
const _approuter = /*#__PURE__*/ _interop_require_default(require("../../client/components/app-router"));
const _serveractionrequestmeta = require("../lib/server-action-request-meta");
const _createinitialrouterstate = require("../../client/components/router-reducer/create-initial-router-state");
const _actionqueue = require("../../shared/lib/router/action-queue");
const _utils = require("../instrumentation/utils");
const _segment = require("../../shared/lib/segment");
const _apprenderprerenderutils = require("./app-render-prerender-utils");
const _prospectiverenderutils = require("./prospective-render-utils");
const _apprenderrenderutils = require("./app-render-render-utils");
const _scheduler = require("../../lib/scheduler");
const _workunitasyncstorageexternal = require("./work-unit-async-storage.external");
const _cachesignal = require("./cache-signal");
const _utils1 = require("../lib/trace/utils");
const _invarianterror = require("../../shared/lib/invariant-error");
require("./clean-async-snapshot.external");
const _constants1 = require("../../lib/constants");
const _createcomponentstylesandscripts = require("./create-component-styles-and-scripts");
const _parseloadertree = require("./parse-loader-tree");
const _resumedatacache = require("../resume-data-cache/resume-data-cache");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const flightDataPathHeadKey = 'h';
function parseRequestHeaders(headers, options) {
    const isDevWarmupRequest = options.isDevWarmup === true;
    // dev warmup requests are treated as prefetch RSC requests
    const isPrefetchRequest = isDevWarmupRequest || headers[_approuterheaders.NEXT_ROUTER_PREFETCH_HEADER.toLowerCase()] !== undefined;
    const isHmrRefresh = headers[_approuterheaders.NEXT_HMR_REFRESH_HEADER.toLowerCase()] !== undefined;
    // dev warmup requests are treated as prefetch RSC requests
    const isRSCRequest = isDevWarmupRequest || headers[_approuterheaders.RSC_HEADER.toLowerCase()] !== undefined;
    const shouldProvideFlightRouterState = isRSCRequest && (!isPrefetchRequest || !options.isRoutePPREnabled);
    const flightRouterState = shouldProvideFlightRouterState ? (0, _parseandvalidateflightrouterstate.parseAndValidateFlightRouterState)(headers[_approuterheaders.NEXT_ROUTER_STATE_TREE_HEADER.toLowerCase()]) : undefined;
    const csp = headers['content-security-policy'] || headers['content-security-policy-report-only'];
    const nonce = typeof csp === 'string' ? (0, _getscriptnoncefromheader.getScriptNonceFromHeader)(csp) : undefined;
    return {
        flightRouterState,
        isPrefetchRequest,
        isHmrRefresh,
        isRSCRequest,
        isDevWarmupRequest,
        nonce
    };
}
function createNotFoundLoaderTree(loaderTree) {
    // Align the segment with parallel-route-default in next-app-loader
    const components = loaderTree[2];
    return [
        '',
        {
            children: [
                _segment.PAGE_SEGMENT_KEY,
                {},
                {
                    page: components['not-found']
                }
            ]
        },
        components
    ];
}
/**
 * Returns a function that parses the dynamic segment and return the associated value.
 */ function makeGetDynamicParamFromSegment(params, pagePath, fallbackRouteParams) {
    return function getDynamicParamFromSegment(// [slug] / [[slug]] / [...slug]
    segment) {
        const segmentParam = (0, _getsegmentparam.getSegmentParam)(segment);
        if (!segmentParam) {
            return null;
        }
        const key = segmentParam.param;
        let value = params[key];
        if (fallbackRouteParams && fallbackRouteParams.has(segmentParam.param)) {
            value = fallbackRouteParams.get(segmentParam.param);
        } else if (Array.isArray(value)) {
            value = value.map((i)=>encodeURIComponent(i));
        } else if (typeof value === 'string') {
            value = encodeURIComponent(value);
        }
        if (!value) {
            const isCatchall = segmentParam.type === 'catchall';
            const isOptionalCatchall = segmentParam.type === 'optional-catchall';
            if (isCatchall || isOptionalCatchall) {
                const dynamicParamType = _getshortdynamicparamtype.dynamicParamTypes[segmentParam.type];
                // handle the case where an optional catchall does not have a value,
                // e.g. `/dashboard/[[...slug]]` when requesting `/dashboard`
                if (isOptionalCatchall) {
                    return {
                        param: key,
                        value: null,
                        type: dynamicParamType,
                        treeSegment: [
                            key,
                            '',
                            dynamicParamType
                        ]
                    };
                }
                // handle the case where a catchall or optional catchall does not have a value,
                // e.g. `/foo/bar/hello` and `@slot/[...catchall]` or `@slot/[[...catchall]]` is matched
                value = pagePath.split('/')// remove the first empty string
                .slice(1)// replace any dynamic params with the actual values
                .flatMap((pathSegment)=>{
                    const param = (0, _routeregex.parseParameter)(pathSegment);
                    // if the segment matches a param, return the param value
                    // otherwise, it's a static segment, so just return that
                    return params[param.key] ?? param.key;
                });
                return {
                    param: key,
                    value,
                    type: dynamicParamType,
                    // This value always has to be a string.
                    treeSegment: [
                        key,
                        value.join('/'),
                        dynamicParamType
                    ]
                };
            }
        }
        const type = (0, _getshortdynamicparamtype.getShortDynamicParamType)(segmentParam.type);
        return {
            param: key,
            // The value that is passed to user code.
            value: value,
            // The value that is rendered in the router tree.
            treeSegment: [
                key,
                Array.isArray(value) ? value.join('/') : value,
                type
            ],
            type: type
        };
    };
}
function NonIndex({ ctx }) {
    const is404Page = ctx.pagePath === '/404';
    const isInvalidStatusCode = typeof ctx.res.statusCode === 'number' && ctx.res.statusCode > 400;
    if (is404Page || isInvalidStatusCode) {
        return /*#__PURE__*/ (0, _jsxruntime.jsx)("meta", {
            name: "robots",
            content: "noindex"
        });
    }
    return null;
}
/**
 * This is used by server actions & client-side navigations to generate RSC data from a client-side request.
 * This function is only called on "dynamic" requests (ie, there wasn't already a static response).
 * It uses request headers (namely `Next-Router-State-Tree`) to determine where to start rendering.
 */ async function generateDynamicRSCPayload(ctx, options) {
    // Flight data that is going to be passed to the browser.
    // Currently a single item array but in the future multiple patches might be combined in a single request.
    // We initialize `flightData` to an empty string because the client router knows how to tolerate
    // it (treating it as an MPA navigation). The only time this function wouldn't generate flight data
    // is for server actions, if the server action handler instructs this function to skip it. When the server
    // action reducer sees a falsy value, it'll simply resolve the action with no data.
    let flightData = '';
    const { componentMod: { tree: loaderTree, createServerSearchParamsForMetadata, createServerParamsForMetadata, createMetadataComponents, MetadataBoundary, ViewportBoundary }, getDynamicParamFromSegment, appUsingSizeAdjustment, query, requestId, flightRouterState, workStore, url } = ctx;
    if (!(options == null ? void 0 : options.skipFlight)) {
        const preloadCallbacks = [];
        const searchParams = createServerSearchParamsForMetadata(query, workStore);
        const [MetadataTree, getMetadataReady] = createMetadataComponents({
            tree: loaderTree,
            searchParams,
            metadataContext: (0, _metadatacontext.createTrackedMetadataContext)(url.pathname, ctx.renderOpts, workStore),
            getDynamicParamFromSegment,
            appUsingSizeAdjustment,
            createServerParamsForMetadata,
            workStore,
            MetadataBoundary,
            ViewportBoundary
        });
        flightData = (await (0, _walktreewithflightrouterstate.walkTreeWithFlightRouterState)({
            ctx,
            createSegmentPath: (child)=>child,
            loaderTreeToFilter: loaderTree,
            parentParams: {},
            flightRouterState,
            isFirst: true,
            // For flight, render metadata inside leaf page
            rscPayloadHead: /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.default.Fragment, {
                children: [
                    /*#__PURE__*/ (0, _jsxruntime.jsx)(NonIndex, {
                        ctx: ctx
                    }),
                    /*#__PURE__*/ (0, _jsxruntime.jsx)(MetadataTree, {}, requestId)
                ]
            }, flightDataPathHeadKey),
            injectedCSS: new Set(),
            injectedJS: new Set(),
            injectedFontPreloadTags: new Set(),
            rootLayoutIncluded: false,
            getMetadataReady,
            preloadCallbacks
        })).map((path)=>path.slice(1)) // remove the '' (root) segment
        ;
    }
    // If we have an action result, then this is a server action response.
    // We can rely on this because `ActionResult` will always be a promise, even if
    // the result is falsey.
    if (options == null ? void 0 : options.actionResult) {
        return {
            a: options.actionResult,
            f: flightData,
            b: ctx.renderOpts.buildId
        };
    }
    // Otherwise, it's a regular RSC response.
    return {
        b: ctx.renderOpts.buildId,
        f: flightData,
        S: workStore.isStaticGeneration
    };
}
function createErrorContext(ctx, renderSource) {
    return {
        routerKind: 'App Router',
        routePath: ctx.pagePath,
        routeType: ctx.isAction ? 'action' : 'render',
        renderSource,
        revalidateReason: (0, _utils.getRevalidateReason)(ctx.workStore)
    };
}
/**
 * Produces a RenderResult containing the Flight data for the given request. See
 * `generateDynamicRSCPayload` for information on the contents of the render result.
 */ async function generateDynamicFlightRenderResult(req, ctx, requestStore, options) {
    const renderOpts = ctx.renderOpts;
    function onFlightDataRenderError(err) {
        return renderOpts.onInstrumentationRequestError == null ? void 0 : renderOpts.onInstrumentationRequestError.call(renderOpts, err, req, createErrorContext(ctx, 'react-server-components-payload'));
    }
    const onError = (0, _createerrorhandler.createFlightReactServerErrorHandler)(!!renderOpts.dev, onFlightDataRenderError);
    const RSCPayload = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(requestStore, generateDynamicRSCPayload, ctx, options);
    if (// We only want this behavior when running `next dev`
    renderOpts.dev && // We only want this behavior when we have React's dev builds available
    process.env.NODE_ENV === 'development' && // We only have a Prerender environment for projects opted into dynamicIO
    renderOpts.experimental.dynamicIO) {
        const [resolveValidation, validationOutlet] = createValidationOutlet();
        RSCPayload._validation = validationOutlet;
        spawnDynamicValidationInDev(resolveValidation, ctx.componentMod.tree, ctx, false, ctx.clientReferenceManifest, ctx.workStore.route, requestStore).catch(resolveValidation) // avoid unhandled rejections and a forever hanging promise
        ;
    }
    // For app dir, use the bundled version of Flight server renderer (renderToReadableStream)
    // which contains the subset React.
    const flightReadableStream = _workunitasyncstorageexternal.workUnitAsyncStorage.run(requestStore, ctx.componentMod.renderToReadableStream, RSCPayload, ctx.clientReferenceManifest.clientModules, {
        onError,
        temporaryReferences: options == null ? void 0 : options.temporaryReferences
    });
    return new _flightrenderresult.FlightRenderResult(flightReadableStream, {
        fetchMetrics: ctx.workStore.fetchMetrics
    });
}
/**
 * Performs a "warmup" render of the RSC payload for a given route. This function is called by the server
 * prior to an actual render request in Dev mode only. It's purpose is to fill caches so the actual render
 * can accurately log activity in the right render context (Prerender vs Render).
 *
 * At the moment this implementation is mostly a fork of generateDynamicFlightRenderResult
 */ async function warmupDevRender(req, ctx) {
    const renderOpts = ctx.renderOpts;
    if (!renderOpts.dev) {
        throw new _invarianterror.InvariantError('generateDynamicFlightRenderResult should never be called in `next start` mode.');
    }
    function onFlightDataRenderError(err) {
        return renderOpts.onInstrumentationRequestError == null ? void 0 : renderOpts.onInstrumentationRequestError.call(renderOpts, err, req, createErrorContext(ctx, 'react-server-components-payload'));
    }
    const onError = (0, _createerrorhandler.createFlightReactServerErrorHandler)(true, onFlightDataRenderError);
    // We're doing a dev warmup, so we should create a new resume data cache so
    // we can fill it.
    const prerenderResumeDataCache = (0, _resumedatacache.createPrerenderResumeDataCache)();
    const renderController = new AbortController();
    const prerenderController = new AbortController();
    const cacheSignal = new _cachesignal.CacheSignal();
    const prerenderStore = {
        type: 'prerender',
        phase: 'render',
        implicitTags: [],
        renderSignal: renderController.signal,
        controller: prerenderController,
        cacheSignal,
        dynamicTracking: null,
        revalidate: _constants1.INFINITE_CACHE,
        expire: _constants1.INFINITE_CACHE,
        stale: _constants1.INFINITE_CACHE,
        tags: [],
        prerenderResumeDataCache
    };
    const rscPayload = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(prerenderStore, generateDynamicRSCPayload, ctx);
    // For app dir, use the bundled version of Flight server renderer (renderToReadableStream)
    // which contains the subset React.
    _workunitasyncstorageexternal.workUnitAsyncStorage.run(prerenderStore, ctx.componentMod.renderToReadableStream, rscPayload, ctx.clientReferenceManifest.clientModules, {
        onError,
        signal: renderController.signal
    });
    // Wait for all caches to be finished filling
    await cacheSignal.cacheReady();
    // We unset the cache so any late over-run renders aren't able to write into this cache
    prerenderStore.prerenderResumeDataCache = null;
    // Abort the render
    renderController.abort();
    // We don't really want to return a result here but the stack of functions
    // that calls into renderToHTML... expects a result. We should refactor this to
    // lift the warmup pathway outside of renderToHTML... but for now this suffices
    return new _flightrenderresult.FlightRenderResult('', {
        fetchMetrics: ctx.workStore.fetchMetrics,
        devRenderResumeDataCache: (0, _resumedatacache.createRenderResumeDataCache)(prerenderResumeDataCache)
    });
}
/**
 * Crawlers will inadvertently think the canonicalUrl in the RSC payload should be crawled
 * when our intention is to just seed the router state with the current URL.
 * This function splits up the pathname so that we can later join it on
 * when we're ready to consume the path.
 */ function prepareInitialCanonicalUrl(url) {
    return (url.pathname + url.search).split('/');
}
// This is the data necessary to render <AppRouter /> when no SSR errors are encountered
async function getRSCPayload(tree, ctx, is404) {
    const injectedCSS = new Set();
    const injectedJS = new Set();
    const injectedFontPreloadTags = new Set();
    let missingSlots;
    // We only track missing parallel slots in development
    if (process.env.NODE_ENV === 'development') {
        missingSlots = new Set();
    }
    const { getDynamicParamFromSegment, query, appUsingSizeAdjustment, componentMod: { GlobalError, createServerSearchParamsForMetadata, createServerParamsForMetadata, createMetadataComponents, MetadataBoundary, ViewportBoundary }, url, workStore } = ctx;
    const initialTree = (0, _createflightrouterstatefromloadertree.createFlightRouterStateFromLoaderTree)(tree, getDynamicParamFromSegment, query);
    const searchParams = createServerSearchParamsForMetadata(query, workStore);
    const [MetadataTree, getMetadataReady] = createMetadataComponents({
        tree,
        errorType: is404 ? 'not-found' : undefined,
        searchParams,
        metadataContext: (0, _metadatacontext.createTrackedMetadataContext)(url.pathname, ctx.renderOpts, workStore),
        getDynamicParamFromSegment,
        appUsingSizeAdjustment,
        createServerParamsForMetadata,
        workStore,
        MetadataBoundary,
        ViewportBoundary
    });
    const preloadCallbacks = [];
    const seedData = await (0, _createcomponenttree.createComponentTree)({
        ctx,
        createSegmentPath: (child)=>child,
        loaderTree: tree,
        parentParams: {},
        firstItem: true,
        injectedCSS,
        injectedJS,
        injectedFontPreloadTags,
        rootLayoutIncluded: false,
        getMetadataReady,
        missingSlots,
        preloadCallbacks,
        authInterrupts: ctx.renderOpts.experimental.authInterrupts
    });
    // When the `vary` response header is present with `Next-URL`, that means there's a chance
    // it could respond differently if there's an interception route. We provide this information
    // to `AppRouter` so that it can properly seed the prefetch cache with a prefix, if needed.
    const varyHeader = ctx.res.getHeader('vary');
    const couldBeIntercepted = typeof varyHeader === 'string' && varyHeader.includes(_approuterheaders.NEXT_URL);
    const initialHead = /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.default.Fragment, {
        children: [
            /*#__PURE__*/ (0, _jsxruntime.jsx)(NonIndex, {
                ctx: ctx
            }),
            /*#__PURE__*/ (0, _jsxruntime.jsx)(MetadataTree, {}, ctx.requestId)
        ]
    }, flightDataPathHeadKey);
    const globalErrorStyles = await getGlobalErrorStyles(tree, ctx);
    // Assume the head we're rendering contains only partial data if PPR is
    // enabled and this is a statically generated response. This is used by the
    // client Segment Cache after a prefetch to determine if it can skip the
    // second request to fill in the dynamic data.
    //
    // See similar comment in create-component-tree.tsx for more context.
    const isPossiblyPartialHead = workStore.isStaticGeneration && ctx.renderOpts.experimental.isRoutePPREnabled === true;
    return {
        // See the comment above the `Preloads` component (below) for why this is part of the payload
        P: /*#__PURE__*/ (0, _jsxruntime.jsx)(Preloads, {
            preloadCallbacks: preloadCallbacks
        }),
        b: ctx.renderOpts.buildId,
        p: ctx.assetPrefix,
        c: prepareInitialCanonicalUrl(url),
        i: !!couldBeIntercepted,
        f: [
            [
                initialTree,
                seedData,
                initialHead,
                isPossiblyPartialHead
            ]
        ],
        m: missingSlots,
        G: [
            GlobalError,
            globalErrorStyles
        ],
        s: typeof ctx.renderOpts.postponed === 'string',
        S: workStore.isStaticGeneration
    };
}
/**
 * Preload calls (such as `ReactDOM.preloadStyle` and `ReactDOM.preloadFont`) need to be called during rendering
 * in order to create the appropriate preload tags in the DOM, otherwise they're a no-op. Since we invoke
 * renderToReadableStream with a function that returns component props rather than a component itself, we use
 * this component to "render  " the preload calls.
 */ function Preloads({ preloadCallbacks }) {
    preloadCallbacks.forEach((preloadFn)=>preloadFn());
    return null;
}
// This is the data necessary to render <AppRouter /> when an error state is triggered
async function getErrorRSCPayload(tree, ctx, errorType) {
    const { getDynamicParamFromSegment, query, appUsingSizeAdjustment, componentMod: { GlobalError, createServerSearchParamsForMetadata, createServerParamsForMetadata, createMetadataComponents, MetadataBoundary, ViewportBoundary }, url, requestId, workStore } = ctx;
    const searchParams = createServerSearchParamsForMetadata(query, workStore);
    const [MetadataTree] = createMetadataComponents({
        tree,
        searchParams,
        // We create an untracked metadata context here because we can't postpone
        // again during the error render.
        metadataContext: (0, _metadatacontext.createMetadataContext)(url.pathname, ctx.renderOpts),
        errorType,
        getDynamicParamFromSegment,
        appUsingSizeAdjustment,
        createServerParamsForMetadata,
        workStore,
        MetadataBoundary,
        ViewportBoundary
    });
    const initialHead = /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.default.Fragment, {
        children: [
            /*#__PURE__*/ (0, _jsxruntime.jsx)(NonIndex, {
                ctx: ctx
            }),
            /*#__PURE__*/ (0, _jsxruntime.jsx)(MetadataTree, {}, requestId),
            process.env.NODE_ENV === 'development' && /*#__PURE__*/ (0, _jsxruntime.jsx)("meta", {
                name: "next-error",
                content: "not-found"
            })
        ]
    }, flightDataPathHeadKey);
    const initialTree = (0, _createflightrouterstatefromloadertree.createFlightRouterStateFromLoaderTree)(tree, getDynamicParamFromSegment, query);
    // For metadata notFound error there's no global not found boundary on top
    // so we create a not found page with AppRouter
    const initialSeedData = [
        initialTree[0],
        /*#__PURE__*/ (0, _jsxruntime.jsxs)("html", {
            id: "__next_error__",
            children: [
                /*#__PURE__*/ (0, _jsxruntime.jsx)("head", {}),
                /*#__PURE__*/ (0, _jsxruntime.jsx)("body", {})
            ]
        }),
        {},
        null,
        false
    ];
    const globalErrorStyles = await getGlobalErrorStyles(tree, ctx);
    const isPossiblyPartialHead = workStore.isStaticGeneration && ctx.renderOpts.experimental.isRoutePPREnabled === true;
    return {
        b: ctx.renderOpts.buildId,
        p: ctx.assetPrefix,
        c: prepareInitialCanonicalUrl(url),
        m: undefined,
        i: false,
        f: [
            [
                initialTree,
                initialSeedData,
                initialHead,
                isPossiblyPartialHead
            ]
        ],
        G: [
            GlobalError,
            globalErrorStyles
        ],
        s: typeof ctx.renderOpts.postponed === 'string',
        S: workStore.isStaticGeneration
    };
}
// This component must run in an SSR context. It will render the RSC root component
function App({ reactServerStream, preinitScripts, clientReferenceManifest, nonce, ServerInsertedHTMLProvider }) {
    preinitScripts();
    const response = _react.default.use((0, _useflightresponse.useFlightStream)(reactServerStream, clientReferenceManifest, nonce));
    const initialState = (0, _createinitialrouterstate.createInitialRouterState)({
        initialFlightData: response.f,
        initialCanonicalUrlParts: response.c,
        // location and initialParallelRoutes are not initialized in the SSR render
        // they are set to an empty map and window.location, respectively during hydration
        initialParallelRoutes: null,
        location: null,
        couldBeIntercepted: response.i,
        postponed: response.s,
        prerendered: response.S
    });
    const actionQueue = (0, _actionqueue.createMutableActionQueue)(initialState);
    const { HeadManagerContext } = require('../../shared/lib/head-manager-context.shared-runtime');
    return /*#__PURE__*/ (0, _jsxruntime.jsx)(HeadManagerContext.Provider, {
        value: {
            appDir: true,
            nonce
        },
        children: /*#__PURE__*/ (0, _jsxruntime.jsx)(ServerInsertedHTMLProvider, {
            children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_approuter.default, {
                actionQueue: actionQueue,
                globalErrorComponentAndStyles: response.G,
                assetPrefix: response.p
            })
        })
    });
}
// @TODO our error stream should be probably just use the same root component. But it was previously
// different I don't want to figure out if that is meaningful at this time so just keeping the behavior
// consistent for now.
function AppWithoutContext({ reactServerStream, preinitScripts, clientReferenceManifest, nonce }) {
    preinitScripts();
    const response = _react.default.use((0, _useflightresponse.useFlightStream)(reactServerStream, clientReferenceManifest, nonce));
    const initialState = (0, _createinitialrouterstate.createInitialRouterState)({
        initialFlightData: response.f,
        initialCanonicalUrlParts: response.c,
        // location and initialParallelRoutes are not initialized in the SSR render
        // they are set to an empty map and window.location, respectively during hydration
        initialParallelRoutes: null,
        location: null,
        couldBeIntercepted: response.i,
        postponed: response.s,
        prerendered: response.S
    });
    const actionQueue = (0, _actionqueue.createMutableActionQueue)(initialState);
    return /*#__PURE__*/ (0, _jsxruntime.jsx)(_approuter.default, {
        actionQueue: actionQueue,
        globalErrorComponentAndStyles: response.G,
        assetPrefix: response.p
    });
}
async function renderToHTMLOrFlightImpl(req, res, url, pagePath, query, renderOpts, workStore, parsedRequestHeaders, requestEndedState, postponedState, implicitTags, serverComponentsHmrCache) {
    const isNotFoundPath = pagePath === '/404';
    if (isNotFoundPath) {
        res.statusCode = 404;
    }
    // A unique request timestamp used by development to ensure that it's
    // consistent and won't change during this request. This is important to
    // avoid that resources can be deduped by React Float if the same resource is
    // rendered or preloaded multiple times: `<link href="a.css?v={Date.now()}"/>`.
    const requestTimestamp = Date.now();
    const { serverActionsManifest, ComponentMod, nextFontManifest, serverActions, assetPrefix = '', enableTainting } = renderOpts;
    // We need to expose the bundled `require` API globally for
    // react-server-dom-webpack. This is a hack until we find a better way.
    if (ComponentMod.__next_app__) {
        const instrumented = (0, _clientcomponentrendererlogger.wrapClientComponentLoader)(ComponentMod);
        // @ts-ignore
        globalThis.__next_require__ = instrumented.require;
        // When we are prerendering if there is a cacheSignal for tracking
        // cache reads we wrap the loadChunk in this tracking. This allows us
        // to treat chunk loading with similar semantics as cache reads to avoid
        // async loading chunks from causing a prerender to abort too early.
        // @ts-ignore
        globalThis.__next_chunk_load__ = (...args)=>{
            const loadingChunk = instrumented.loadChunk(...args);
            trackChunkLoading(loadingChunk);
            return loadingChunk;
        };
    }
    if (process.env.NODE_ENV === 'development') {
        // reset isr status at start of request
        const { pathname } = new URL(req.url || '/', 'http://n');
        renderOpts.setAppIsrStatus == null ? void 0 : renderOpts.setAppIsrStatus.call(renderOpts, pathname, null);
    }
    if (// The type check here ensures that `req` is correctly typed, and the
    // environment variable check provides dead code elimination.
    process.env.NEXT_RUNTIME !== 'edge' && (0, _helpers.isNodeNextRequest)(req)) {
        req.originalRequest.on('end', ()=>{
            requestEndedState.ended = true;
            if ('performance' in globalThis) {
                const metrics = (0, _clientcomponentrendererlogger.getClientComponentLoaderMetrics)({
                    reset: true
                });
                if (metrics) {
                    (0, _tracer.getTracer)().startSpan(_constants.NextNodeServerSpan.clientComponentLoading, {
                        startTime: metrics.clientComponentLoadStart,
                        attributes: {
                            'next.clientComponentLoadCount': metrics.clientComponentLoadCount,
                            'next.span_type': _constants.NextNodeServerSpan.clientComponentLoading
                        }
                    }).end(metrics.clientComponentLoadStart + metrics.clientComponentLoadTimes);
                }
            }
        });
    }
    const metadata = {};
    const appUsingSizeAdjustment = !!(nextFontManifest == null ? void 0 : nextFontManifest.appUsingSizeAdjust);
    // TODO: fix this typescript
    const clientReferenceManifest = renderOpts.clientReferenceManifest;
    const serverModuleMap = (0, _actionutils.createServerModuleMap)({
        serverActionsManifest
    });
    (0, _encryptionutils.setReferenceManifestsSingleton)({
        page: workStore.page,
        clientReferenceManifest,
        serverActionsManifest,
        serverModuleMap
    });
    ComponentMod.patchFetch();
    // Pull out the hooks/references from the component.
    const { tree: loaderTree, taintObjectReference } = ComponentMod;
    if (enableTainting) {
        taintObjectReference('Do not pass process.env to client components since it will leak sensitive data', process.env);
    }
    workStore.fetchMetrics = [];
    metadata.fetchMetrics = workStore.fetchMetrics;
    // don't modify original query object
    query = {
        ...query
    };
    (0, _internalutils.stripInternalQueries)(query);
    const { flightRouterState, isPrefetchRequest, isRSCRequest, isDevWarmupRequest, isHmrRefresh, nonce } = parsedRequestHeaders;
    /**
   * The metadata items array created in next-app-loader with all relevant information
   * that we need to resolve the final metadata.
   */ let requestId;
    if (process.env.NEXT_RUNTIME === 'edge') {
        requestId = crypto.randomUUID();
    } else {
        requestId = require('next/dist/compiled/nanoid').nanoid();
    }
    /**
   * Dynamic parameters. E.g. when you visit `/dashboard/vercel` which is rendered by `/dashboard/[slug]` the value will be {"slug": "vercel"}.
   */ const params = renderOpts.params ?? {};
    const { isStaticGeneration, fallbackRouteParams } = workStore;
    const getDynamicParamFromSegment = makeGetDynamicParamFromSegment(params, pagePath, fallbackRouteParams);
    const isActionRequest = (0, _serveractionrequestmeta.getServerActionRequestMetadata)(req).isServerAction;
    const ctx = {
        componentMod: ComponentMod,
        url,
        renderOpts,
        workStore,
        parsedRequestHeaders,
        getDynamicParamFromSegment,
        query,
        isPrefetch: isPrefetchRequest,
        isAction: isActionRequest,
        requestTimestamp,
        appUsingSizeAdjustment,
        flightRouterState,
        requestId,
        pagePath,
        clientReferenceManifest,
        assetPrefix,
        isNotFoundPath,
        nonce,
        res
    };
    (0, _tracer.getTracer)().setRootSpanAttribute('next.route', pagePath);
    if (isStaticGeneration) {
        // We're either building or revalidating. In either case we need to
        // prerender our page rather than render it.
        const prerenderToStreamWithTracing = (0, _tracer.getTracer)().wrap(_constants.AppRenderSpan.getBodyResult, {
            spanName: `prerender route (app) ${pagePath}`,
            attributes: {
                'next.route': pagePath
            }
        }, prerenderToStream);
        const response = await prerenderToStreamWithTracing(req, res, ctx, metadata, workStore, loaderTree, implicitTags);
        // If we're debugging partial prerendering, print all the dynamic API accesses
        // that occurred during the render.
        // @TODO move into renderToStream function
        if (response.dynamicAccess && (0, _dynamicrendering.accessedDynamicData)(response.dynamicAccess) && renderOpts.isDebugDynamicAccesses) {
            (0, _log.warn)('The following dynamic usage was detected:');
            for (const access of (0, _dynamicrendering.formatDynamicAPIAccesses)(response.dynamicAccess)){
                (0, _log.warn)(access);
            }
        }
        // If we encountered any unexpected errors during build we fail the
        // prerendering phase and the build.
        if (response.digestErrorsMap.size) {
            const buildFailingError = response.digestErrorsMap.values().next().value;
            if (buildFailingError) throw buildFailingError;
        }
        // Pick first userland SSR error, which is also not a RSC error.
        if (response.ssrErrors.length) {
            const buildFailingError = response.ssrErrors.find((err)=>(0, _createerrorhandler.isUserLandError)(err));
            if (buildFailingError) throw buildFailingError;
        }
        const options = {
            metadata
        };
        // If we have pending revalidates, wait until they are all resolved.
        if (workStore.pendingRevalidates || workStore.pendingRevalidateWrites || workStore.revalidatedTags) {
            var _workStore_incrementalCache;
            options.waitUntil = Promise.all([
                (_workStore_incrementalCache = workStore.incrementalCache) == null ? void 0 : _workStore_incrementalCache.revalidateTag(workStore.revalidatedTags || []),
                ...Object.values(workStore.pendingRevalidates || {}),
                ...workStore.pendingRevalidateWrites || []
            ]);
        }
        if (response.collectedTags) {
            metadata.fetchTags = response.collectedTags.join(',');
        }
        // Let the client router know how long to keep the cached entry around.
        const staleHeader = String(response.collectedStale);
        res.setHeader(_approuterheaders.NEXT_ROUTER_STALE_TIME_HEADER, staleHeader);
        metadata.headers ??= {};
        metadata.headers[_approuterheaders.NEXT_ROUTER_STALE_TIME_HEADER] = staleHeader;
        // If force static is specifically set to false, we should not revalidate
        // the page.
        if (workStore.forceStatic === false || response.collectedRevalidate === 0) {
            metadata.revalidate = 0;
        } else {
            // Copy the revalidation value onto the render result metadata.
            metadata.revalidate = response.collectedRevalidate >= _constants1.INFINITE_CACHE ? false : response.collectedRevalidate;
        }
        // provide bailout info for debugging
        if (metadata.revalidate === 0) {
            metadata.staticBailoutInfo = {
                description: workStore.dynamicUsageDescription,
                stack: workStore.dynamicUsageStack
            };
        }
        return new _renderresult.default(await (0, _nodewebstreamshelper.streamToString)(response.stream), options);
    } else {
        // We're rendering dynamically
        const renderResumeDataCache = renderOpts.devRenderResumeDataCache ?? (postponedState == null ? void 0 : postponedState.renderResumeDataCache);
        const requestStore = (0, _requeststore.createRequestStoreForRender)(req, res, url, implicitTags, renderOpts.onUpdateCookies, renderOpts.previewProps, isHmrRefresh, serverComponentsHmrCache, renderResumeDataCache);
        if (process.env.NODE_ENV === 'development' && renderOpts.setAppIsrStatus && // The type check here ensures that `req` is correctly typed, and the
        // environment variable check provides dead code elimination.
        process.env.NEXT_RUNTIME !== 'edge' && (0, _helpers.isNodeNextRequest)(req) && !isDevWarmupRequest) {
            const setAppIsrStatus = renderOpts.setAppIsrStatus;
            req.originalRequest.on('end', ()=>{
                if (!requestStore.usedDynamic && !workStore.forceDynamic) {
                    // only node can be ISR so we only need to update the status here
                    const { pathname } = new URL(req.url || '/', 'http://n');
                    setAppIsrStatus(pathname, true);
                }
            });
        }
        if (isDevWarmupRequest) {
            return warmupDevRender(req, ctx);
        } else if (isRSCRequest) {
            return generateDynamicFlightRenderResult(req, ctx, requestStore);
        }
        const renderToStreamWithTracing = (0, _tracer.getTracer)().wrap(_constants.AppRenderSpan.getBodyResult, {
            spanName: `render route (app) ${pagePath}`,
            attributes: {
                'next.route': pagePath
            }
        }, renderToStream);
        let formState = null;
        if (isActionRequest) {
            // For action requests, we handle them differently with a special render result.
            const actionRequestResult = await (0, _actionhandler.handleAction)({
                req,
                res,
                ComponentMod,
                serverModuleMap,
                generateFlight: generateDynamicFlightRenderResult,
                workStore,
                requestStore,
                serverActions,
                ctx
            });
            if (actionRequestResult) {
                if (actionRequestResult.type === 'not-found') {
                    const notFoundLoaderTree = createNotFoundLoaderTree(loaderTree);
                    res.statusCode = 404;
                    const stream = await renderToStreamWithTracing(requestStore, req, res, ctx, workStore, notFoundLoaderTree, formState, postponedState);
                    return new _renderresult.default(stream, {
                        metadata
                    });
                } else if (actionRequestResult.type === 'done') {
                    if (actionRequestResult.result) {
                        actionRequestResult.result.assignMetadata(metadata);
                        return actionRequestResult.result;
                    } else if (actionRequestResult.formState) {
                        formState = actionRequestResult.formState;
                    }
                }
            }
        }
        const options = {
            metadata
        };
        const stream = await renderToStreamWithTracing(requestStore, req, res, ctx, workStore, loaderTree, formState, postponedState);
        // If we have pending revalidates, wait until they are all resolved.
        if (workStore.pendingRevalidates || workStore.pendingRevalidateWrites || workStore.revalidatedTags) {
            var _workStore_incrementalCache1;
            options.waitUntil = Promise.all([
                (_workStore_incrementalCache1 = workStore.incrementalCache) == null ? void 0 : _workStore_incrementalCache1.revalidateTag(workStore.revalidatedTags || []),
                ...Object.values(workStore.pendingRevalidates || {}),
                ...workStore.pendingRevalidateWrites || []
            ]);
        }
        // Create the new render result for the response.
        return new _renderresult.default(stream, options);
    }
}
const renderToHTMLOrFlight = (req, res, pagePath, query, fallbackRouteParams, renderOpts, serverComponentsHmrCache, isDevWarmup)=>{
    if (!req.url) {
        throw new Error('Invalid URL');
    }
    const url = (0, _parserelativeurl.parseRelativeUrl)(req.url, undefined, false);
    // We read these values from the request object as, in certain cases,
    // base-server will strip them to opt into different rendering behavior.
    const parsedRequestHeaders = parseRequestHeaders(req.headers, {
        isDevWarmup,
        isRoutePPREnabled: renderOpts.experimental.isRoutePPREnabled === true
    });
    const { isPrefetchRequest } = parsedRequestHeaders;
    const requestEndedState = {
        ended: false
    };
    let postponedState = null;
    // If provided, the postpone state should be parsed so it can be provided to
    // React.
    if (typeof renderOpts.postponed === 'string') {
        if (fallbackRouteParams) {
            throw new _invarianterror.InvariantError('postponed state should not be provided when fallback params are provided');
        }
        postponedState = (0, _postponedstate.parsePostponedState)(renderOpts.postponed, renderOpts.params);
    }
    if ((postponedState == null ? void 0 : postponedState.renderResumeDataCache) && renderOpts.devRenderResumeDataCache) {
        throw new _invarianterror.InvariantError('postponed state and dev warmup immutable resume data cache should not be provided together');
    }
    const implicitTags = (0, _implicittags.getImplicitTags)(renderOpts.routeModule.definition.page, url, fallbackRouteParams);
    const workStore = (0, _workstore.createWorkStore)({
        page: renderOpts.routeModule.definition.page,
        fallbackRouteParams,
        renderOpts,
        requestEndedState,
        // @TODO move to workUnitStore of type Request
        isPrefetchRequest
    });
    return _workasyncstorageexternal.workAsyncStorage.run(workStore, // The function to run
    renderToHTMLOrFlightImpl, // all of it's args
    req, res, url, pagePath, query, renderOpts, workStore, parsedRequestHeaders, requestEndedState, postponedState, implicitTags, serverComponentsHmrCache);
};
async function renderToStream(requestStore, req, res, ctx, workStore, tree, formState, postponedState) {
    const renderOpts = ctx.renderOpts;
    const ComponentMod = renderOpts.ComponentMod;
    // TODO: fix this typescript
    const clientReferenceManifest = renderOpts.clientReferenceManifest;
    const { ServerInsertedHTMLProvider, renderServerInsertedHTML } = (0, _serverinsertedhtml.createServerInsertedHTML)();
    const tracingMetadata = (0, _utils1.getTracedMetadata)((0, _tracer.getTracer)().getTracePropagationData(), renderOpts.experimental.clientTraceMetadata);
    const polyfills = renderOpts.buildManifest.polyfillFiles.filter((polyfill)=>polyfill.endsWith('.js') && !polyfill.endsWith('.module.js')).map((polyfill)=>{
        var _renderOpts_subresourceIntegrityManifest;
        return {
            src: `${ctx.assetPrefix}/_next/${polyfill}${(0, _getassetquerystring.getAssetQueryString)(ctx, false)}`,
            integrity: (_renderOpts_subresourceIntegrityManifest = renderOpts.subresourceIntegrityManifest) == null ? void 0 : _renderOpts_subresourceIntegrityManifest[polyfill],
            crossOrigin: renderOpts.crossOrigin,
            noModule: true,
            nonce: ctx.nonce
        };
    });
    const [preinitScripts, bootstrapScript] = (0, _requiredscripts.getRequiredScripts)(renderOpts.buildManifest, // Why is assetPrefix optional on renderOpts?
    // @TODO make it default empty string on renderOpts and get rid of it from ctx
    ctx.assetPrefix, renderOpts.crossOrigin, renderOpts.subresourceIntegrityManifest, (0, _getassetquerystring.getAssetQueryString)(ctx, true), ctx.nonce, renderOpts.page);
    const reactServerErrorsByDigest = new Map();
    const silenceLogger = false;
    function onHTMLRenderRSCError(err) {
        return renderOpts.onInstrumentationRequestError == null ? void 0 : renderOpts.onInstrumentationRequestError.call(renderOpts, err, req, createErrorContext(ctx, 'react-server-components'));
    }
    const serverComponentsErrorHandler = (0, _createerrorhandler.createHTMLReactServerErrorHandler)(!!renderOpts.dev, !!renderOpts.nextExport, reactServerErrorsByDigest, silenceLogger, onHTMLRenderRSCError);
    function onHTMLRenderSSRError(err) {
        return renderOpts.onInstrumentationRequestError == null ? void 0 : renderOpts.onInstrumentationRequestError.call(renderOpts, err, req, createErrorContext(ctx, 'server-rendering'));
    }
    const allCapturedErrors = [];
    const htmlRendererErrorHandler = (0, _createerrorhandler.createHTMLErrorHandler)(!!renderOpts.dev, !!renderOpts.nextExport, reactServerErrorsByDigest, allCapturedErrors, silenceLogger, onHTMLRenderSSRError);
    let reactServerResult = null;
    const setHeader = res.setHeader.bind(res);
    try {
        if (// We only want this behavior when running `next dev`
        renderOpts.dev && // We only want this behavior when we have React's dev builds available
        process.env.NODE_ENV === 'development' && // Edge routes never prerender so we don't have a Prerender environment for anything in edge runtime
        process.env.NEXT_RUNTIME !== 'edge' && // We only have a Prerender environment for projects opted into dynamicIO
        renderOpts.experimental.dynamicIO) {
            // This is a dynamic render. We don't do dynamic tracking because we're not prerendering
            const RSCPayload = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(requestStore, getRSCPayload, tree, ctx, res.statusCode === 404);
            const [resolveValidation, validationOutlet] = createValidationOutlet();
            RSCPayload._validation = validationOutlet;
            const reactServerStream = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(requestStore, _apprenderrenderutils.scheduleInSequentialTasks, ()=>{
                requestStore.prerenderPhase = true;
                return ComponentMod.renderToReadableStream(RSCPayload, clientReferenceManifest.clientModules, {
                    onError: serverComponentsErrorHandler,
                    environmentName: ()=>requestStore.prerenderPhase === true ? 'Prerender' : 'Server',
                    filterStackFrame (url, _functionName) {
                        // The default implementation filters out <anonymous> stack frames
                        // but we want to retain them because current Server Components and
                        // built-in Components in parent stacks don't have source location.
                        return !url.startsWith('node:') && !url.includes('node_modules');
                    }
                });
            }, ()=>{
                requestStore.prerenderPhase = false;
            });
            spawnDynamicValidationInDev(resolveValidation, tree, ctx, res.statusCode === 404, clientReferenceManifest, workStore.route, requestStore).catch(resolveValidation) // avoid unhandled rejections and a forever hanging promise
            ;
            reactServerResult = new _apprenderprerenderutils.ReactServerResult(reactServerStream);
        } else {
            // This is a dynamic render. We don't do dynamic tracking because we're not prerendering
            const RSCPayload = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(requestStore, getRSCPayload, tree, ctx, res.statusCode === 404);
            reactServerResult = new _apprenderprerenderutils.ReactServerResult(_workunitasyncstorageexternal.workUnitAsyncStorage.run(requestStore, ComponentMod.renderToReadableStream, RSCPayload, clientReferenceManifest.clientModules, {
                onError: serverComponentsErrorHandler
            }));
        }
        // React doesn't start rendering synchronously but we want the RSC render to have a chance to start
        // before we begin SSR rendering because we want to capture any available preload headers so we tick
        // one task before continuing
        await (0, _scheduler.waitAtLeastOneReactRenderTask)();
        // If provided, the postpone state should be parsed as JSON so it can be
        // provided to React.
        if (typeof renderOpts.postponed === 'string') {
            if ((postponedState == null ? void 0 : postponedState.type) === _postponedstate.DynamicState.DATA) {
                // We have a complete HTML Document in the prerender but we need to
                // still include the new server component render because it was not included
                // in the static prelude.
                const inlinedReactServerDataStream = (0, _useflightresponse.createInlinedDataReadableStream)(reactServerResult.tee(), ctx.nonce, formState);
                return (0, _nodewebstreamshelper.chainStreams)(inlinedReactServerDataStream, (0, _nodewebstreamshelper.createDocumentClosingStream)());
            } else if (postponedState) {
                // We assume we have dynamic HTML requiring a resume render to complete
                const postponed = (0, _postponedstate.getPostponedFromState)(postponedState);
                const resume = require('react-dom/server.edge').resume;
                const htmlStream = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(requestStore, resume, /*#__PURE__*/ (0, _jsxruntime.jsx)(App, {
                    reactServerStream: reactServerResult.tee(),
                    preinitScripts: preinitScripts,
                    clientReferenceManifest: clientReferenceManifest,
                    ServerInsertedHTMLProvider: ServerInsertedHTMLProvider,
                    nonce: ctx.nonce
                }), postponed, {
                    onError: htmlRendererErrorHandler,
                    nonce: ctx.nonce
                });
                const getServerInsertedHTML = (0, _makegetserverinsertedhtml.makeGetServerInsertedHTML)({
                    polyfills,
                    renderServerInsertedHTML,
                    serverCapturedErrors: allCapturedErrors,
                    basePath: renderOpts.basePath,
                    tracingMetadata: tracingMetadata
                });
                return await (0, _nodewebstreamshelper.continueDynamicHTMLResume)(htmlStream, {
                    inlinedDataStream: (0, _useflightresponse.createInlinedDataReadableStream)(reactServerResult.consume(), ctx.nonce, formState),
                    getServerInsertedHTML
                });
            }
        }
        // This is a regular dynamic render
        const renderToReadableStream = require('react-dom/server.edge').renderToReadableStream;
        const htmlStream = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(requestStore, renderToReadableStream, /*#__PURE__*/ (0, _jsxruntime.jsx)(App, {
            reactServerStream: reactServerResult.tee(),
            preinitScripts: preinitScripts,
            clientReferenceManifest: clientReferenceManifest,
            ServerInsertedHTMLProvider: ServerInsertedHTMLProvider,
            nonce: ctx.nonce
        }), {
            onError: htmlRendererErrorHandler,
            nonce: ctx.nonce,
            onHeaders: (headers)=>{
                headers.forEach((value, key)=>{
                    setHeader(key, value);
                });
            },
            maxHeadersLength: renderOpts.reactMaxHeadersLength,
            // When debugging the static shell, client-side rendering should be
            // disabled to prevent blanking out the page.
            bootstrapScripts: renderOpts.isDebugStaticShell ? [] : [
                bootstrapScript
            ],
            formState
        });
        const getServerInsertedHTML = (0, _makegetserverinsertedhtml.makeGetServerInsertedHTML)({
            polyfills,
            renderServerInsertedHTML,
            serverCapturedErrors: allCapturedErrors,
            basePath: renderOpts.basePath,
            tracingMetadata: tracingMetadata
        });
        /**
     * Rules of Static & Dynamic HTML:
     *
     *    1.) We must generate static HTML unless the caller explicitly opts
     *        in to dynamic HTML support.
     *
     *    2.) If dynamic HTML support is requested, we must honor that request
     *        or throw an error. It is the sole responsibility of the caller to
     *        ensure they aren't e.g. requesting dynamic HTML for an AMP page.
     *
     * These rules help ensure that other existing features like request caching,
     * coalescing, and ISR continue working as intended.
     */ const generateStaticHTML = renderOpts.supportsDynamicResponse !== true;
        const validateRootLayout = renderOpts.dev;
        return await (0, _nodewebstreamshelper.continueFizzStream)(htmlStream, {
            inlinedDataStream: (0, _useflightresponse.createInlinedDataReadableStream)(reactServerResult.consume(), ctx.nonce, formState),
            isStaticGeneration: generateStaticHTML,
            getServerInsertedHTML,
            serverInsertedHTMLToHead: true,
            validateRootLayout
        });
    } catch (err) {
        if ((0, _staticgenerationbailout.isStaticGenBailoutError)(err) || typeof err === 'object' && err !== null && 'message' in err && typeof err.message === 'string' && err.message.includes('https://nextjs.org/docs/advanced-features/static-html-export')) {
            // Ensure that "next dev" prints the red error overlay
            throw err;
        }
        // If a bailout made it to this point, it means it wasn't wrapped inside
        // a suspense boundary.
        const shouldBailoutToCSR = (0, _bailouttocsr.isBailoutToCSRError)(err);
        if (shouldBailoutToCSR) {
            const stack = (0, _formatservererror.getStackWithoutErrorMessage)(err);
            (0, _log.error)(`${err.reason} should be wrapped in a suspense boundary at page "${ctx.pagePath}". Read more: https://nextjs.org/docs/messages/missing-suspense-with-csr-bailout\n${stack}`);
            throw err;
        }
        let errorType;
        if ((0, _httpaccessfallback.isHTTPAccessFallbackError)(err)) {
            res.statusCode = (0, _httpaccessfallback.getAccessFallbackHTTPStatus)(err);
            errorType = (0, _httpaccessfallback.getAccessFallbackErrorTypeByStatus)(res.statusCode);
        } else if ((0, _redirecterror.isRedirectError)(err)) {
            errorType = 'redirect';
            res.statusCode = (0, _redirect.getRedirectStatusCodeFromError)(err);
            const redirectUrl = (0, _addpathprefix.addPathPrefix)((0, _redirect.getURLFromRedirectError)(err), renderOpts.basePath);
            // If there were mutable cookies set, we need to set them on the
            // response.
            const headers = new Headers();
            if ((0, _requestcookies.appendMutableCookies)(headers, requestStore.mutableCookies)) {
                setHeader('set-cookie', Array.from(headers.values()));
            }
            setHeader('location', redirectUrl);
        } else if (!shouldBailoutToCSR) {
            res.statusCode = 500;
        }
        const [errorPreinitScripts, errorBootstrapScript] = (0, _requiredscripts.getRequiredScripts)(renderOpts.buildManifest, ctx.assetPrefix, renderOpts.crossOrigin, renderOpts.subresourceIntegrityManifest, (0, _getassetquerystring.getAssetQueryString)(ctx, false), ctx.nonce, '/_not-found/page');
        const errorRSCPayload = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(requestStore, getErrorRSCPayload, tree, ctx, errorType);
        const errorServerStream = _workunitasyncstorageexternal.workUnitAsyncStorage.run(requestStore, ComponentMod.renderToReadableStream, errorRSCPayload, clientReferenceManifest.clientModules, {
            onError: serverComponentsErrorHandler
        });
        if (reactServerResult === null) {
            // We errored when we did not have an RSC stream to read from. This is not just a render
            // error, we need to throw early
            throw err;
        }
        try {
            const fizzStream = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(requestStore, _nodewebstreamshelper.renderToInitialFizzStream, {
                ReactDOMServer: require('react-dom/server.edge'),
                element: /*#__PURE__*/ (0, _jsxruntime.jsx)(AppWithoutContext, {
                    reactServerStream: errorServerStream,
                    preinitScripts: errorPreinitScripts,
                    clientReferenceManifest: clientReferenceManifest,
                    nonce: ctx.nonce
                }),
                streamOptions: {
                    nonce: ctx.nonce,
                    // Include hydration scripts in the HTML
                    bootstrapScripts: [
                        errorBootstrapScript
                    ],
                    formState
                }
            });
            /**
       * Rules of Static & Dynamic HTML:
       *
       *    1.) We must generate static HTML unless the caller explicitly opts
       *        in to dynamic HTML support.
       *
       *    2.) If dynamic HTML support is requested, we must honor that request
       *        or throw an error. It is the sole responsibility of the caller to
       *        ensure they aren't e.g. requesting dynamic HTML for an AMP page.
       *
       * These rules help ensure that other existing features like request caching,
       * coalescing, and ISR continue working as intended.
       */ const generateStaticHTML = renderOpts.supportsDynamicResponse !== true;
            const validateRootLayout = renderOpts.dev;
            return await (0, _nodewebstreamshelper.continueFizzStream)(fizzStream, {
                inlinedDataStream: (0, _useflightresponse.createInlinedDataReadableStream)(// This is intentionally using the readable datastream from the
                // main render rather than the flight data from the error page
                // render
                reactServerResult.consume(), ctx.nonce, formState),
                isStaticGeneration: generateStaticHTML,
                getServerInsertedHTML: (0, _makegetserverinsertedhtml.makeGetServerInsertedHTML)({
                    polyfills,
                    renderServerInsertedHTML,
                    serverCapturedErrors: [],
                    basePath: renderOpts.basePath,
                    tracingMetadata: tracingMetadata
                }),
                serverInsertedHTMLToHead: true,
                validateRootLayout
            });
        } catch (finalErr) {
            if (process.env.NODE_ENV === 'development' && (0, _httpaccessfallback.isHTTPAccessFallbackError)(finalErr)) {
                const { bailOnRootNotFound } = require('../../client/components/dev-root-http-access-fallback-boundary');
                bailOnRootNotFound();
            }
            throw finalErr;
        }
    }
}
function createValidationOutlet() {
    let resolveValidation;
    let outlet = new Promise((resolve)=>{
        resolveValidation = resolve;
    });
    return [
        resolveValidation,
        outlet
    ];
}
async function spawnDynamicValidationInDev(resolveValidation, tree, ctx, isNotFound, clientReferenceManifest, route, requestStore) {
    const { componentMod: ComponentMod } = ctx;
    // Prerender controller represents the lifetime of the prerender.
    // It will be aborted when a Task is complete or a synchronously aborting
    // API is called. Notably during cache-filling renders this does not actually
    // terminate the render itself which will continue until all caches are filled
    const initialServerPrerenderController = new AbortController();
    // This controller represents the lifetime of the React render call. Notably
    // during the cache-filling render it is different from the prerender controller
    // because we don't want to end the react render until all caches are filled.
    const initialServerRenderController = new AbortController();
    const cacheSignal = new _cachesignal.CacheSignal();
    const prerenderResumeDataCache = (0, _resumedatacache.createPrerenderResumeDataCache)();
    const initialServerPrerenderStore = {
        type: 'prerender',
        phase: 'render',
        implicitTags: [],
        renderSignal: initialServerRenderController.signal,
        controller: initialServerPrerenderController,
        cacheSignal,
        dynamicTracking: null,
        revalidate: _constants1.INFINITE_CACHE,
        expire: _constants1.INFINITE_CACHE,
        stale: _constants1.INFINITE_CACHE,
        tags: [],
        prerenderResumeDataCache
    };
    const initialClientController = new AbortController();
    const initialClientPrerenderStore = {
        type: 'prerender',
        phase: 'render',
        implicitTags: [],
        renderSignal: initialClientController.signal,
        controller: initialClientController,
        cacheSignal,
        dynamicTracking: null,
        revalidate: _constants1.INFINITE_CACHE,
        expire: _constants1.INFINITE_CACHE,
        stale: _constants1.INFINITE_CACHE,
        tags: [],
        prerenderResumeDataCache
    };
    // We're not going to use the result of this render because the only time it could be used
    // is if it completes in a microtask and that's likely very rare for any non-trivial app
    const firstAttemptRSCPayload = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(initialServerPrerenderStore, getRSCPayload, tree, ctx, isNotFound);
    let initialServerStream;
    try {
        initialServerStream = _workunitasyncstorageexternal.workUnitAsyncStorage.run(initialServerPrerenderStore, ComponentMod.renderToReadableStream, firstAttemptRSCPayload, clientReferenceManifest.clientModules, {
            onError: (err)=>{
                const digest = (0, _createerrorhandler.getDigestForWellKnownError)(err);
                if (digest) {
                    return digest;
                }
                if (initialServerPrerenderController.signal.aborted || initialServerRenderController.signal.aborted) {
                    // The render aborted before this error was handled which indicates
                    // the error is caused by unfinished components within the render
                    return;
                } else if (process.env.NEXT_DEBUG_BUILD || process.env.__NEXT_VERBOSE_LOGGING) {
                    (0, _prospectiverenderutils.printDebugThrownValueForProspectiveRender)(err, route);
                }
            },
            signal: initialServerRenderController.signal
        });
    } catch (err) {
        if (initialServerPrerenderController.signal.aborted || initialServerRenderController.signal.aborted) {
        // These are expected errors that might error the prerender. we ignore them.
        } else if (process.env.NEXT_DEBUG_BUILD || process.env.__NEXT_VERBOSE_LOGGING) {
            // We don't normally log these errors because we are going to retry anyway but
            // it can be useful for debugging Next.js itself to get visibility here when needed
            (0, _prospectiverenderutils.printDebugThrownValueForProspectiveRender)(err, route);
        }
    }
    const { ServerInsertedHTMLProvider } = (0, _serverinsertedhtml.createServerInsertedHTML)();
    const nonce = '1';
    if (initialServerStream) {
        const [warmupStream, renderStream] = initialServerStream.tee();
        initialServerStream = null;
        // Before we attempt the SSR initial render we need to ensure all client modules
        // are already loaded.
        await warmFlightResponse(warmupStream, clientReferenceManifest);
        const prerender = require('react-dom/static.edge').prerender;
        const pendingInitialClientResult = _workunitasyncstorageexternal.workUnitAsyncStorage.run(initialClientPrerenderStore, prerender, /*#__PURE__*/ (0, _jsxruntime.jsx)(App, {
            reactServerStream: renderStream,
            preinitScripts: ()=>{},
            clientReferenceManifest: clientReferenceManifest,
            ServerInsertedHTMLProvider: ServerInsertedHTMLProvider,
            nonce: nonce
        }), {
            signal: initialClientController.signal,
            onError: (err)=>{
                const digest = (0, _createerrorhandler.getDigestForWellKnownError)(err);
                if (digest) {
                    return digest;
                }
                if (initialClientController.signal.aborted) {
                // These are expected errors that might error the prerender. we ignore them.
                } else if (process.env.NEXT_DEBUG_BUILD || process.env.__NEXT_VERBOSE_LOGGING) {
                    // We don't normally log these errors because we are going to retry anyway but
                    // it can be useful for debugging Next.js itself to get visibility here when needed
                    (0, _prospectiverenderutils.printDebugThrownValueForProspectiveRender)(err, route);
                }
            }
        });
        pendingInitialClientResult.catch((err)=>{
            if (initialClientController.signal.aborted) {
            // We aborted the render normally and can ignore this error
            } else {
                // We're going to retry to so we normally would suppress this error but
                // when verbose logging is on we print it
                if (process.env.__NEXT_VERBOSE_LOGGING) {
                    (0, _prospectiverenderutils.printDebugThrownValueForProspectiveRender)(err, route);
                }
            }
        });
    }
    await cacheSignal.cacheReady();
    // It is important that we abort the SSR render first to avoid
    // connection closed errors from having an incomplete RSC stream
    initialClientController.abort();
    initialServerRenderController.abort();
    initialServerPrerenderController.abort();
    // We've now filled caches and triggered any inadvertent sync bailouts
    // due to lazy module initialization. We can restart our render to capture results
    const finalServerController = new AbortController();
    const serverDynamicTracking = (0, _dynamicrendering.createDynamicTrackingState)(false);
    const finalServerPrerenderStore = {
        type: 'prerender',
        phase: 'render',
        implicitTags: [],
        renderSignal: finalServerController.signal,
        controller: finalServerController,
        // During the final prerender we don't need to track cache access so we omit the signal
        cacheSignal: null,
        dynamicTracking: serverDynamicTracking,
        revalidate: _constants1.INFINITE_CACHE,
        expire: _constants1.INFINITE_CACHE,
        stale: _constants1.INFINITE_CACHE,
        tags: [],
        prerenderResumeDataCache
    };
    const finalClientController = new AbortController();
    const clientDynamicTracking = (0, _dynamicrendering.createDynamicTrackingState)(false);
    const dynamicValidation = (0, _dynamicrendering.createDynamicValidationState)();
    const finalClientPrerenderStore = {
        type: 'prerender',
        phase: 'render',
        implicitTags: [],
        renderSignal: finalClientController.signal,
        controller: finalClientController,
        // During the final prerender we don't need to track cache access so we omit the signal
        cacheSignal: null,
        dynamicTracking: clientDynamicTracking,
        revalidate: _constants1.INFINITE_CACHE,
        expire: _constants1.INFINITE_CACHE,
        stale: _constants1.INFINITE_CACHE,
        tags: [],
        prerenderResumeDataCache
    };
    const finalServerPayload = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(finalServerPrerenderStore, getRSCPayload, tree, ctx, isNotFound);
    const serverPrerenderStreamResult = await (0, _apprenderprerenderutils.prerenderServerWithPhases)(finalServerController.signal, ()=>_workunitasyncstorageexternal.workUnitAsyncStorage.run(finalServerPrerenderStore, ComponentMod.renderToReadableStream, finalServerPayload, clientReferenceManifest.clientModules, {
            onError: (err)=>{
                if (finalServerController.signal.aborted && (0, _dynamicrendering.isPrerenderInterruptedError)(err)) {
                    return err.digest;
                }
                return (0, _createerrorhandler.getDigestForWellKnownError)(err);
            },
            signal: finalServerController.signal
        }), ()=>{
        finalServerController.abort();
    });
    const serverPhasedStream = serverPrerenderStreamResult.asPhasedStream();
    try {
        const prerender = require('react-dom/static.edge').prerender;
        await (0, _apprenderprerenderutils.prerenderClientWithPhases)(()=>_workunitasyncstorageexternal.workUnitAsyncStorage.run(finalClientPrerenderStore, prerender, /*#__PURE__*/ (0, _jsxruntime.jsx)(App, {
                reactServerStream: serverPhasedStream,
                preinitScripts: ()=>{},
                clientReferenceManifest: clientReferenceManifest,
                ServerInsertedHTMLProvider: ServerInsertedHTMLProvider,
                nonce: ctx.nonce
            }), {
                signal: finalClientController.signal,
                onError: (err, errorInfo)=>{
                    if ((0, _dynamicrendering.isPrerenderInterruptedError)(err) || finalClientController.signal.aborted) {
                        requestStore.usedDynamic = true;
                        const componentStack = errorInfo.componentStack;
                        if (typeof componentStack === 'string') {
                            (0, _dynamicrendering.trackAllowedDynamicAccess)(route, componentStack, dynamicValidation, serverDynamicTracking, clientDynamicTracking);
                        }
                        return;
                    }
                    return (0, _createerrorhandler.getDigestForWellKnownError)(err);
                }
            }), ()=>{
            finalClientController.abort();
            serverPhasedStream.assertExhausted();
        });
    } catch (err) {
        if ((0, _dynamicrendering.isPrerenderInterruptedError)(err) || finalClientController.signal.aborted) {
        // we don't have a root because the abort errored in the root. We can just ignore this error
        } else {
            // This error is something else and should bubble up
            throw err;
        }
    }
    function LogDynamicValidation() {
        try {
            (0, _dynamicrendering.throwIfDisallowedDynamic)(route, dynamicValidation, serverDynamicTracking, clientDynamicTracking);
        } catch  {}
        return null;
    }
    resolveValidation(/*#__PURE__*/ (0, _jsxruntime.jsx)(LogDynamicValidation, {}));
}
/**
 * Determines whether we should generate static flight data.
 */ function shouldGenerateStaticFlightData(workStore) {
    const { fallbackRouteParams, isStaticGeneration } = workStore;
    if (!isStaticGeneration) return false;
    if (fallbackRouteParams && fallbackRouteParams.size > 0) {
        return false;
    }
    return true;
}
async function prerenderToStream(req, res, ctx, metadata, workStore, tree, implicitTags) {
    // When prerendering formState is always null. We still include it
    // because some shared APIs expect a formState value and this is slightly
    // more explicit than making it an optional function argument
    const formState = null;
    const renderOpts = ctx.renderOpts;
    const ComponentMod = renderOpts.ComponentMod;
    // TODO: fix this typescript
    const clientReferenceManifest = renderOpts.clientReferenceManifest;
    const fallbackRouteParams = workStore.fallbackRouteParams;
    const { ServerInsertedHTMLProvider, renderServerInsertedHTML } = (0, _serverinsertedhtml.createServerInsertedHTML)();
    const tracingMetadata = (0, _utils1.getTracedMetadata)((0, _tracer.getTracer)().getTracePropagationData(), renderOpts.experimental.clientTraceMetadata);
    const polyfills = renderOpts.buildManifest.polyfillFiles.filter((polyfill)=>polyfill.endsWith('.js') && !polyfill.endsWith('.module.js')).map((polyfill)=>{
        var _renderOpts_subresourceIntegrityManifest;
        return {
            src: `${ctx.assetPrefix}/_next/${polyfill}${(0, _getassetquerystring.getAssetQueryString)(ctx, false)}`,
            integrity: (_renderOpts_subresourceIntegrityManifest = renderOpts.subresourceIntegrityManifest) == null ? void 0 : _renderOpts_subresourceIntegrityManifest[polyfill],
            crossOrigin: renderOpts.crossOrigin,
            noModule: true,
            nonce: ctx.nonce
        };
    });
    const [preinitScripts, bootstrapScript] = (0, _requiredscripts.getRequiredScripts)(renderOpts.buildManifest, // Why is assetPrefix optional on renderOpts?
    // @TODO make it default empty string on renderOpts and get rid of it from ctx
    ctx.assetPrefix, renderOpts.crossOrigin, renderOpts.subresourceIntegrityManifest, (0, _getassetquerystring.getAssetQueryString)(ctx, true), ctx.nonce, renderOpts.page);
    const reactServerErrorsByDigest = new Map();
    // We don't report errors during prerendering through our instrumentation hooks
    const silenceLogger = !!renderOpts.experimental.isRoutePPREnabled;
    function onHTMLRenderRSCError(err) {
        return renderOpts.onInstrumentationRequestError == null ? void 0 : renderOpts.onInstrumentationRequestError.call(renderOpts, err, req, createErrorContext(ctx, 'react-server-components'));
    }
    const serverComponentsErrorHandler = (0, _createerrorhandler.createHTMLReactServerErrorHandler)(!!renderOpts.dev, !!renderOpts.nextExport, reactServerErrorsByDigest, silenceLogger, onHTMLRenderRSCError);
    function onHTMLRenderSSRError(err) {
        return renderOpts.onInstrumentationRequestError == null ? void 0 : renderOpts.onInstrumentationRequestError.call(renderOpts, err, req, createErrorContext(ctx, 'server-rendering'));
    }
    const allCapturedErrors = [];
    const htmlRendererErrorHandler = (0, _createerrorhandler.createHTMLErrorHandler)(!!renderOpts.dev, !!renderOpts.nextExport, reactServerErrorsByDigest, allCapturedErrors, silenceLogger, onHTMLRenderSSRError);
    let reactServerPrerenderResult = null;
    const setHeader = (name, value)=>{
        res.setHeader(name, value);
        metadata.headers ??= {};
        metadata.headers[name] = res.getHeader(name);
        return res;
    };
    let prerenderStore = null;
    try {
        if (renderOpts.experimental.dynamicIO) {
            if (renderOpts.experimental.isRoutePPREnabled) {
                /**
         * dynamicIO with PPR
         *
         * The general approach is to render the RSC stream first allowing any cache reads to resolve.
         * Once we have settled all cache reads we restart the render and abort after a single Task.
         *
         * Unlike with the non PPR case we can't synchronously abort the render when a dynamic API is used
         * during the initial render because we need to ensure all caches can be filled as part of the initial Task
         * and a synchronous abort might prevent us from filling all caches.
         *
         * Once the render is complete we allow the SSR render to finish and use a combination of the postponed state
         * and the reactServerIsDynamic value to determine how to treat the resulting render
         */ // Prerender controller represents the lifetime of the prerender.
                // It will be aborted when a Task is complete or a synchronously aborting
                // API is called. Notably during cache-filling renders this does not actually
                // terminate the render itself which will continue until all caches are filled
                const initialServerPrerenderController = new AbortController();
                // This controller represents the lifetime of the React render call. Notably
                // during the cache-filling render it is different from the prerender controller
                // because we don't want to end the react render until all caches are filled.
                const initialServerRenderController = new AbortController();
                // The cacheSignal helps us track whether caches are still filling or we are ready
                // to cut the render off.
                const cacheSignal = new _cachesignal.CacheSignal();
                // The resume data cache here should use a fresh instance as it's
                // performing a fresh prerender. If we get to implementing the
                // prerendering of an already prerendered page, we should use the passed
                // resume data cache instead.
                const prerenderResumeDataCache = (0, _resumedatacache.createPrerenderResumeDataCache)();
                const initialServerPrerenderStore = prerenderStore = {
                    type: 'prerender',
                    phase: 'render',
                    implicitTags: implicitTags,
                    renderSignal: initialServerRenderController.signal,
                    controller: initialServerPrerenderController,
                    cacheSignal,
                    dynamicTracking: null,
                    revalidate: _constants1.INFINITE_CACHE,
                    expire: _constants1.INFINITE_CACHE,
                    stale: _constants1.INFINITE_CACHE,
                    tags: [
                        ...implicitTags
                    ],
                    prerenderResumeDataCache
                };
                // We're not going to use the result of this render because the only time it could be used
                // is if it completes in a microtask and that's likely very rare for any non-trivial app
                const initialServerPayload = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(initialServerPrerenderStore, getRSCPayload, tree, ctx, res.statusCode === 404);
                const pendingInitialServerResult = _workunitasyncstorageexternal.workUnitAsyncStorage.run(initialServerPrerenderStore, ComponentMod.prerender, initialServerPayload, clientReferenceManifest.clientModules, {
                    onError: (err)=>{
                        const digest = (0, _createerrorhandler.getDigestForWellKnownError)(err);
                        if (digest) {
                            return digest;
                        }
                        if (initialServerPrerenderController.signal.aborted) {
                            // The render aborted before this error was handled which indicates
                            // the error is caused by unfinished components within the render
                            return;
                        } else if (process.env.NEXT_DEBUG_BUILD || process.env.__NEXT_VERBOSE_LOGGING) {
                            (0, _prospectiverenderutils.printDebugThrownValueForProspectiveRender)(err, workStore.route);
                        }
                    },
                    // we don't care to track postpones during the prospective render because we need
                    // to always do a final render anyway
                    onPostpone: undefined,
                    // We don't want to stop rendering until the cacheSignal is complete so we pass
                    // a different signal to this render call than is used by dynamic APIs to signify
                    // transitioning out of the prerender environment
                    signal: initialServerRenderController.signal
                });
                await cacheSignal.cacheReady();
                initialServerRenderController.abort();
                initialServerPrerenderController.abort();
                let initialServerResult;
                try {
                    initialServerResult = await (0, _apprenderprerenderutils.createReactServerPrerenderResult)(pendingInitialServerResult);
                } catch (err) {
                    if (initialServerRenderController.signal.aborted || initialServerPrerenderController.signal.aborted) {
                    // These are expected errors that might error the prerender. we ignore them.
                    } else if (process.env.NEXT_DEBUG_BUILD || process.env.__NEXT_VERBOSE_LOGGING) {
                        // We don't normally log these errors because we are going to retry anyway but
                        // it can be useful for debugging Next.js itself to get visibility here when needed
                        (0, _prospectiverenderutils.printDebugThrownValueForProspectiveRender)(err, workStore.route);
                    }
                }
                if (initialServerResult) {
                    // Before we attempt the SSR initial render we need to ensure all client modules
                    // are already loaded.
                    await warmFlightResponse(initialServerResult.asStream(), clientReferenceManifest);
                    const initialClientController = new AbortController();
                    const initialClientPrerenderStore = {
                        type: 'prerender',
                        phase: 'render',
                        implicitTags: implicitTags,
                        renderSignal: initialClientController.signal,
                        controller: initialClientController,
                        cacheSignal: null,
                        dynamicTracking: null,
                        revalidate: _constants1.INFINITE_CACHE,
                        expire: _constants1.INFINITE_CACHE,
                        stale: _constants1.INFINITE_CACHE,
                        tags: [
                            ...implicitTags
                        ],
                        prerenderResumeDataCache
                    };
                    const prerender = require('react-dom/static.edge').prerender;
                    await (0, _apprenderprerenderutils.prerenderAndAbortInSequentialTasks)(()=>_workunitasyncstorageexternal.workUnitAsyncStorage.run(initialClientPrerenderStore, prerender, /*#__PURE__*/ (0, _jsxruntime.jsx)(App, {
                            reactServerStream: initialServerResult.asUnclosingStream(),
                            preinitScripts: preinitScripts,
                            clientReferenceManifest: clientReferenceManifest,
                            ServerInsertedHTMLProvider: ServerInsertedHTMLProvider,
                            nonce: ctx.nonce
                        }), {
                            signal: initialClientController.signal,
                            onError: (err)=>{
                                const digest = (0, _createerrorhandler.getDigestForWellKnownError)(err);
                                if (digest) {
                                    return digest;
                                }
                                if (initialClientController.signal.aborted) {
                                // These are expected errors that might error the prerender. we ignore them.
                                } else if (process.env.NEXT_DEBUG_BUILD || process.env.__NEXT_VERBOSE_LOGGING) {
                                    // We don't normally log these errors because we are going to retry anyway but
                                    // it can be useful for debugging Next.js itself to get visibility here when needed
                                    (0, _prospectiverenderutils.printDebugThrownValueForProspectiveRender)(err, workStore.route);
                                }
                            },
                            // When debugging the static shell, client-side rendering should be
                            // disabled to prevent blanking out the page.
                            bootstrapScripts: renderOpts.isDebugStaticShell ? [] : [
                                bootstrapScript
                            ]
                        }), ()=>{
                        initialClientController.abort();
                    }).catch((err)=>{
                        if (initialServerRenderController.signal.aborted || (0, _dynamicrendering.isPrerenderInterruptedError)(err)) {
                        // These are expected errors that might error the prerender. we ignore them.
                        } else if (process.env.NEXT_DEBUG_BUILD || process.env.__NEXT_VERBOSE_LOGGING) {
                            // We don't normally log these errors because we are going to retry anyway but
                            // it can be useful for debugging Next.js itself to get visibility here when needed
                            (0, _prospectiverenderutils.printDebugThrownValueForProspectiveRender)(err, workStore.route);
                        }
                    });
                }
                let serverIsDynamic = false;
                const finalServerController = new AbortController();
                const serverDynamicTracking = (0, _dynamicrendering.createDynamicTrackingState)(renderOpts.isDebugDynamicAccesses);
                const finalRenderPrerenderStore = prerenderStore = {
                    type: 'prerender',
                    phase: 'render',
                    implicitTags: implicitTags,
                    renderSignal: finalServerController.signal,
                    controller: finalServerController,
                    // During the final prerender we don't need to track cache access so we omit the signal
                    cacheSignal: null,
                    dynamicTracking: serverDynamicTracking,
                    revalidate: _constants1.INFINITE_CACHE,
                    expire: _constants1.INFINITE_CACHE,
                    stale: _constants1.INFINITE_CACHE,
                    tags: [
                        ...implicitTags
                    ],
                    prerenderResumeDataCache
                };
                const finalAttemptRSCPayload = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(finalRenderPrerenderStore, getRSCPayload, tree, ctx, res.statusCode === 404);
                const reactServerResult = reactServerPrerenderResult = await (0, _apprenderprerenderutils.createReactServerPrerenderResult)((0, _apprenderprerenderutils.prerenderAndAbortInSequentialTasks)(()=>_workunitasyncstorageexternal.workUnitAsyncStorage.run(// The store to scope
                    finalRenderPrerenderStore, // The function to run
                    ComponentMod.prerender, // ... the arguments for the function to run
                    finalAttemptRSCPayload, clientReferenceManifest.clientModules, {
                        onError: (err)=>{
                            if (finalServerController.signal.aborted) {
                                serverIsDynamic = true;
                                return;
                            }
                            return serverComponentsErrorHandler(err);
                        },
                        signal: finalServerController.signal
                    }), ()=>{
                    finalServerController.abort();
                }));
                const clientDynamicTracking = (0, _dynamicrendering.createDynamicTrackingState)(renderOpts.isDebugDynamicAccesses);
                const finalClientController = new AbortController();
                const finalClientPrerenderStore = {
                    type: 'prerender',
                    phase: 'render',
                    implicitTags: implicitTags,
                    renderSignal: finalClientController.signal,
                    controller: finalClientController,
                    // For HTML Generation we don't need to track cache reads (RSC only)
                    cacheSignal: null,
                    dynamicTracking: clientDynamicTracking,
                    revalidate: _constants1.INFINITE_CACHE,
                    expire: _constants1.INFINITE_CACHE,
                    stale: _constants1.INFINITE_CACHE,
                    tags: [
                        ...implicitTags
                    ],
                    prerenderResumeDataCache
                };
                let clientIsDynamic = false;
                let dynamicValidation = (0, _dynamicrendering.createDynamicValidationState)();
                const prerender = require('react-dom/static.edge').prerender;
                let { prelude, postponed } = await (0, _apprenderprerenderutils.prerenderAndAbortInSequentialTasks)(()=>_workunitasyncstorageexternal.workUnitAsyncStorage.run(finalClientPrerenderStore, prerender, /*#__PURE__*/ (0, _jsxruntime.jsx)(App, {
                        reactServerStream: reactServerResult.asUnclosingStream(),
                        preinitScripts: preinitScripts,
                        clientReferenceManifest: clientReferenceManifest,
                        ServerInsertedHTMLProvider: ServerInsertedHTMLProvider,
                        nonce: ctx.nonce
                    }), {
                        signal: finalClientController.signal,
                        onError: (err, errorInfo)=>{
                            if ((0, _dynamicrendering.isPrerenderInterruptedError)(err) || finalClientController.signal.aborted) {
                                clientIsDynamic = true;
                                const componentStack = errorInfo.componentStack;
                                if (typeof componentStack === 'string') {
                                    (0, _dynamicrendering.trackAllowedDynamicAccess)(workStore.route, componentStack, dynamicValidation, serverDynamicTracking, clientDynamicTracking);
                                }
                                return;
                            }
                            return htmlRendererErrorHandler(err, errorInfo);
                        },
                        onHeaders: (headers)=>{
                            headers.forEach((value, key)=>{
                                setHeader(key, value);
                            });
                        },
                        maxHeadersLength: renderOpts.reactMaxHeadersLength,
                        // When debugging the static shell, client-side rendering should be
                        // disabled to prevent blanking out the page.
                        bootstrapScripts: renderOpts.isDebugStaticShell ? [] : [
                            bootstrapScript
                        ]
                    }), ()=>{
                    finalClientController.abort();
                });
                (0, _dynamicrendering.throwIfDisallowedDynamic)(workStore.route, dynamicValidation, serverDynamicTracking, clientDynamicTracking);
                const getServerInsertedHTML = (0, _makegetserverinsertedhtml.makeGetServerInsertedHTML)({
                    polyfills,
                    renderServerInsertedHTML,
                    serverCapturedErrors: allCapturedErrors,
                    basePath: renderOpts.basePath,
                    tracingMetadata: tracingMetadata
                });
                const flightData = await (0, _nodewebstreamshelper.streamToBuffer)(reactServerResult.asStream());
                metadata.flightData = flightData;
                metadata.segmentData = await collectSegmentData(flightData, finalRenderPrerenderStore, ComponentMod, renderOpts);
                if (serverIsDynamic || clientIsDynamic) {
                    if (postponed != null) {
                        // Dynamic HTML case
                        metadata.postponed = await (0, _postponedstate.getDynamicHTMLPostponedState)(postponed, fallbackRouteParams, prerenderResumeDataCache);
                    } else {
                        // Dynamic Data case
                        metadata.postponed = await (0, _postponedstate.getDynamicDataPostponedState)(prerenderResumeDataCache);
                    }
                    reactServerResult.consume();
                    return {
                        digestErrorsMap: reactServerErrorsByDigest,
                        ssrErrors: allCapturedErrors,
                        stream: await (0, _nodewebstreamshelper.continueDynamicPrerender)(prelude, {
                            getServerInsertedHTML
                        }),
                        dynamicAccess: (0, _dynamicrendering.consumeDynamicAccess)(serverDynamicTracking, clientDynamicTracking),
                        // TODO: Should this include the SSR pass?
                        collectedRevalidate: finalRenderPrerenderStore.revalidate,
                        collectedExpire: finalRenderPrerenderStore.expire,
                        collectedStale: finalRenderPrerenderStore.stale,
                        collectedTags: finalRenderPrerenderStore.tags
                    };
                } else {
                    // Static case
                    if (workStore.forceDynamic) {
                        throw new _staticgenerationbailout.StaticGenBailoutError('Invariant: a Page with `dynamic = "force-dynamic"` did not trigger the dynamic pathway. This is a bug in Next.js');
                    }
                    let htmlStream = prelude;
                    if (postponed != null) {
                        // We postponed but nothing dynamic was used. We resume the render now and immediately abort it
                        // so we can set all the postponed boundaries to client render mode before we store the HTML response
                        const resume = require('react-dom/server.edge').resume;
                        // We don't actually want to render anything so we just pass a stream
                        // that never resolves. The resume call is going to abort immediately anyway
                        const foreverStream = new ReadableStream();
                        const resumeStream = await resume(/*#__PURE__*/ (0, _jsxruntime.jsx)(App, {
                            reactServerStream: foreverStream,
                            preinitScripts: ()=>{},
                            clientReferenceManifest: clientReferenceManifest,
                            ServerInsertedHTMLProvider: ServerInsertedHTMLProvider,
                            nonce: ctx.nonce
                        }), JSON.parse(JSON.stringify(postponed)), {
                            signal: (0, _dynamicrendering.createPostponedAbortSignal)('static prerender resume'),
                            onError: htmlRendererErrorHandler,
                            nonce: ctx.nonce
                        });
                        // First we write everything from the prerender, then we write everything from the aborted resume render
                        htmlStream = (0, _nodewebstreamshelper.chainStreams)(prelude, resumeStream);
                    }
                    return {
                        digestErrorsMap: reactServerErrorsByDigest,
                        ssrErrors: allCapturedErrors,
                        stream: await (0, _nodewebstreamshelper.continueStaticPrerender)(htmlStream, {
                            inlinedDataStream: (0, _useflightresponse.createInlinedDataReadableStream)(reactServerResult.consumeAsStream(), ctx.nonce, formState),
                            getServerInsertedHTML
                        }),
                        dynamicAccess: (0, _dynamicrendering.consumeDynamicAccess)(serverDynamicTracking, clientDynamicTracking),
                        // TODO: Should this include the SSR pass?
                        collectedRevalidate: finalRenderPrerenderStore.revalidate,
                        collectedExpire: finalRenderPrerenderStore.expire,
                        collectedStale: finalRenderPrerenderStore.stale,
                        collectedTags: finalRenderPrerenderStore.tags
                    };
                }
            } else {
                /**
         * dynamicIO without PPR
         *
         * The general approach is to render the RSC tree first allowing for any inflight
         * caches to resolve. Once we have settled inflight caches we can check and see if any
         * synchronous dynamic APIs were used. If so we don't need to bother doing anything more
         * because the page will be dynamic on re-render anyway
         *
         * If no sync dynamic APIs were used we then re-render and abort after a single Task.
         * If the render errors we know that the page has some dynamic IO. This assumes and relies
         * upon caches reading from a in process memory cache and resolving in a microtask. While this
         * is true from our own default cache implementation and if you don't exceed our LRU size it
         * might not be true for custom cache implementations.
         *
         * Future implementations can do some different strategies during build like using IPC to
         * synchronously fill caches during this special rendering mode. For now this heuristic should work
         */ const cache = workStore.incrementalCache;
                if (!cache) {
                    throw new Error('Expected incremental cache to exist. This is a bug in Next.js');
                }
                // Prerender controller represents the lifetime of the prerender.
                // It will be aborted when a Task is complete or a synchronously aborting
                // API is called. Notably during cache-filling renders this does not actually
                // terminate the render itself which will continue until all caches are filled
                const initialServerPrerenderController = new AbortController();
                // This controller represents the lifetime of the React render call. Notably
                // during the cache-filling render it is different from the prerender controller
                // because we don't want to end the react render until all caches are filled.
                const initialServerRenderController = new AbortController();
                const cacheSignal = new _cachesignal.CacheSignal();
                const prerenderResumeDataCache = (0, _resumedatacache.createPrerenderResumeDataCache)();
                const initialServerPrerenderStore = prerenderStore = {
                    type: 'prerender',
                    phase: 'render',
                    implicitTags: implicitTags,
                    renderSignal: initialServerRenderController.signal,
                    controller: initialServerPrerenderController,
                    cacheSignal,
                    dynamicTracking: null,
                    revalidate: _constants1.INFINITE_CACHE,
                    expire: _constants1.INFINITE_CACHE,
                    stale: _constants1.INFINITE_CACHE,
                    tags: [
                        ...implicitTags
                    ],
                    prerenderResumeDataCache
                };
                const initialClientController = new AbortController();
                const initialClientPrerenderStore = prerenderStore = {
                    type: 'prerender',
                    phase: 'render',
                    implicitTags: implicitTags,
                    renderSignal: initialClientController.signal,
                    controller: initialClientController,
                    cacheSignal,
                    dynamicTracking: null,
                    revalidate: _constants1.INFINITE_CACHE,
                    expire: _constants1.INFINITE_CACHE,
                    stale: _constants1.INFINITE_CACHE,
                    tags: [
                        ...implicitTags
                    ],
                    prerenderResumeDataCache
                };
                // We're not going to use the result of this render because the only time it could be used
                // is if it completes in a microtask and that's likely very rare for any non-trivial app
                const firstAttemptRSCPayload = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(initialServerPrerenderStore, getRSCPayload, tree, ctx, res.statusCode === 404);
                let initialServerStream;
                try {
                    initialServerStream = _workunitasyncstorageexternal.workUnitAsyncStorage.run(initialServerPrerenderStore, ComponentMod.renderToReadableStream, firstAttemptRSCPayload, clientReferenceManifest.clientModules, {
                        onError: (err)=>{
                            const digest = (0, _createerrorhandler.getDigestForWellKnownError)(err);
                            if (digest) {
                                return digest;
                            }
                            if (initialServerPrerenderController.signal.aborted || initialServerRenderController.signal.aborted) {
                                // The render aborted before this error was handled which indicates
                                // the error is caused by unfinished components within the render
                                return;
                            } else if (process.env.NEXT_DEBUG_BUILD || process.env.__NEXT_VERBOSE_LOGGING) {
                                (0, _prospectiverenderutils.printDebugThrownValueForProspectiveRender)(err, workStore.route);
                            }
                        },
                        signal: initialServerRenderController.signal
                    });
                } catch (err) {
                    if (initialServerPrerenderController.signal.aborted || initialServerRenderController.signal.aborted) {
                    // These are expected errors that might error the prerender. we ignore them.
                    } else if (process.env.NEXT_DEBUG_BUILD || process.env.__NEXT_VERBOSE_LOGGING) {
                        // We don't normally log these errors because we are going to retry anyway but
                        // it can be useful for debugging Next.js itself to get visibility here when needed
                        (0, _prospectiverenderutils.printDebugThrownValueForProspectiveRender)(err, workStore.route);
                    }
                }
                if (initialServerStream) {
                    const [warmupStream, renderStream] = initialServerStream.tee();
                    initialServerStream = null;
                    // Before we attempt the SSR initial render we need to ensure all client modules
                    // are already loaded.
                    await warmFlightResponse(warmupStream, clientReferenceManifest);
                    const prerender = require('react-dom/static.edge').prerender;
                    const pendingInitialClientResult = _workunitasyncstorageexternal.workUnitAsyncStorage.run(initialClientPrerenderStore, prerender, /*#__PURE__*/ (0, _jsxruntime.jsx)(App, {
                        reactServerStream: renderStream,
                        preinitScripts: preinitScripts,
                        clientReferenceManifest: clientReferenceManifest,
                        ServerInsertedHTMLProvider: ServerInsertedHTMLProvider,
                        nonce: ctx.nonce
                    }), {
                        signal: initialClientController.signal,
                        onError: (err)=>{
                            const digest = (0, _createerrorhandler.getDigestForWellKnownError)(err);
                            if (digest) {
                                return digest;
                            }
                            if (initialClientController.signal.aborted) {
                            // These are expected errors that might error the prerender. we ignore them.
                            } else if (process.env.NEXT_DEBUG_BUILD || process.env.__NEXT_VERBOSE_LOGGING) {
                                // We don't normally log these errors because we are going to retry anyway but
                                // it can be useful for debugging Next.js itself to get visibility here when needed
                                (0, _prospectiverenderutils.printDebugThrownValueForProspectiveRender)(err, workStore.route);
                            }
                        },
                        // When debugging the static shell, client-side rendering should be
                        // disabled to prevent blanking out the page.
                        bootstrapScripts: renderOpts.isDebugStaticShell ? [] : [
                            bootstrapScript
                        ]
                    });
                    pendingInitialClientResult.catch((err)=>{
                        if (initialClientController.signal.aborted) {
                        // We aborted the render normally and can ignore this error
                        } else {
                            // We're going to retry to so we normally would suppress this error but
                            // when verbose logging is on we print it
                            if (process.env.__NEXT_VERBOSE_LOGGING) {
                                (0, _prospectiverenderutils.printDebugThrownValueForProspectiveRender)(err, workStore.route);
                            }
                        }
                    });
                }
                await cacheSignal.cacheReady();
                // It is important that we abort the SSR render first to avoid
                // connection closed errors from having an incomplete RSC stream
                initialClientController.abort();
                initialServerRenderController.abort();
                initialServerPrerenderController.abort();
                // We've now filled caches and triggered any inadvertant sync bailouts
                // due to lazy module initialization. We can restart our render to capture results
                let serverIsDynamic = false;
                const finalServerController = new AbortController();
                const serverDynamicTracking = (0, _dynamicrendering.createDynamicTrackingState)(renderOpts.isDebugDynamicAccesses);
                const finalServerPrerenderStore = prerenderStore = {
                    type: 'prerender',
                    phase: 'render',
                    implicitTags: implicitTags,
                    renderSignal: finalServerController.signal,
                    controller: finalServerController,
                    // During the final prerender we don't need to track cache access so we omit the signal
                    cacheSignal: null,
                    dynamicTracking: serverDynamicTracking,
                    revalidate: _constants1.INFINITE_CACHE,
                    expire: _constants1.INFINITE_CACHE,
                    stale: _constants1.INFINITE_CACHE,
                    tags: [
                        ...implicitTags
                    ],
                    prerenderResumeDataCache
                };
                let clientIsDynamic = false;
                const finalClientController = new AbortController();
                const clientDynamicTracking = (0, _dynamicrendering.createDynamicTrackingState)(renderOpts.isDebugDynamicAccesses);
                const dynamicValidation = (0, _dynamicrendering.createDynamicValidationState)();
                const finalClientPrerenderStore = prerenderStore = {
                    type: 'prerender',
                    phase: 'render',
                    implicitTags: implicitTags,
                    renderSignal: finalClientController.signal,
                    controller: finalClientController,
                    // During the final prerender we don't need to track cache access so we omit the signal
                    cacheSignal: null,
                    dynamicTracking: clientDynamicTracking,
                    revalidate: _constants1.INFINITE_CACHE,
                    expire: _constants1.INFINITE_CACHE,
                    stale: _constants1.INFINITE_CACHE,
                    tags: [
                        ...implicitTags
                    ],
                    prerenderResumeDataCache
                };
                const finalServerPayload = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(finalServerPrerenderStore, getRSCPayload, tree, ctx, res.statusCode === 404);
                const serverPrerenderStreamResult = reactServerPrerenderResult = await (0, _apprenderprerenderutils.prerenderServerWithPhases)(finalServerController.signal, ()=>_workunitasyncstorageexternal.workUnitAsyncStorage.run(finalServerPrerenderStore, ComponentMod.renderToReadableStream, finalServerPayload, clientReferenceManifest.clientModules, {
                        onError: (err)=>{
                            if (finalServerController.signal.aborted) {
                                serverIsDynamic = true;
                                if ((0, _dynamicrendering.isPrerenderInterruptedError)(err)) {
                                    return err.digest;
                                }
                                return (0, _createerrorhandler.getDigestForWellKnownError)(err);
                            }
                            return serverComponentsErrorHandler(err);
                        },
                        signal: finalServerController.signal
                    }), ()=>{
                    finalServerController.abort();
                });
                let htmlStream;
                const serverPhasedStream = serverPrerenderStreamResult.asPhasedStream();
                try {
                    const prerender = require('react-dom/static.edge').prerender;
                    const result = await (0, _apprenderprerenderutils.prerenderClientWithPhases)(()=>_workunitasyncstorageexternal.workUnitAsyncStorage.run(finalClientPrerenderStore, prerender, /*#__PURE__*/ (0, _jsxruntime.jsx)(App, {
                            reactServerStream: serverPhasedStream,
                            preinitScripts: preinitScripts,
                            clientReferenceManifest: clientReferenceManifest,
                            ServerInsertedHTMLProvider: ServerInsertedHTMLProvider,
                            nonce: ctx.nonce
                        }), {
                            signal: finalClientController.signal,
                            onError: (err, errorInfo)=>{
                                if ((0, _dynamicrendering.isPrerenderInterruptedError)(err) || finalClientController.signal.aborted) {
                                    clientIsDynamic = true;
                                    const componentStack = errorInfo.componentStack;
                                    if (typeof componentStack === 'string') {
                                        (0, _dynamicrendering.trackAllowedDynamicAccess)(workStore.route, componentStack, dynamicValidation, serverDynamicTracking, clientDynamicTracking);
                                    }
                                    return;
                                }
                                return htmlRendererErrorHandler(err, errorInfo);
                            },
                            // When debugging the static shell, client-side rendering should be
                            // disabled to prevent blanking out the page.
                            bootstrapScripts: renderOpts.isDebugStaticShell ? [] : [
                                bootstrapScript
                            ]
                        }), ()=>{
                        finalClientController.abort();
                        serverPhasedStream.assertExhausted();
                    });
                    htmlStream = result.prelude;
                } catch (err) {
                    if ((0, _dynamicrendering.isPrerenderInterruptedError)(err) || finalClientController.signal.aborted) {
                    // we don't have a root because the abort errored in the root. We can just ignore this error
                    } else {
                        // This error is something else and should bubble up
                        throw err;
                    }
                }
                (0, _dynamicrendering.throwIfDisallowedDynamic)(workStore.route, dynamicValidation, serverDynamicTracking, clientDynamicTracking);
                if (serverIsDynamic || clientIsDynamic) {
                    const dynamicReason = serverIsDynamic ? (0, _dynamicrendering.getFirstDynamicReason)(serverDynamicTracking) : (0, _dynamicrendering.getFirstDynamicReason)(clientDynamicTracking);
                    if (dynamicReason) {
                        throw new _hooksservercontext.DynamicServerError(`Route "${workStore.route}" couldn't be rendered statically because it used \`${dynamicReason}\`. See more info here: https://nextjs.org/docs/messages/next-prerender-data`);
                    } else {
                        throw new _hooksservercontext.DynamicServerError(`Route "${workStore.route}" couldn't be rendered statically it accessed data without explicitly caching it. See more info here: https://nextjs.org/docs/messages/next-prerender-data`);
                    }
                }
                const flightData = await (0, _nodewebstreamshelper.streamToBuffer)(serverPrerenderStreamResult.asStream());
                metadata.flightData = flightData;
                metadata.segmentData = await collectSegmentData(flightData, finalClientPrerenderStore, ComponentMod, renderOpts);
                const getServerInsertedHTML = (0, _makegetserverinsertedhtml.makeGetServerInsertedHTML)({
                    polyfills,
                    renderServerInsertedHTML,
                    serverCapturedErrors: allCapturedErrors,
                    basePath: renderOpts.basePath,
                    tracingMetadata: tracingMetadata
                });
                const validateRootLayout = renderOpts.dev;
                return {
                    digestErrorsMap: reactServerErrorsByDigest,
                    ssrErrors: allCapturedErrors,
                    stream: await (0, _nodewebstreamshelper.continueFizzStream)(htmlStream, {
                        inlinedDataStream: (0, _useflightresponse.createInlinedDataReadableStream)(serverPrerenderStreamResult.asStream(), ctx.nonce, formState),
                        isStaticGeneration: true,
                        getServerInsertedHTML,
                        serverInsertedHTMLToHead: true,
                        validateRootLayout
                    }),
                    dynamicAccess: (0, _dynamicrendering.consumeDynamicAccess)(serverDynamicTracking, clientDynamicTracking),
                    // TODO: Should this include the SSR pass?
                    collectedRevalidate: finalServerPrerenderStore.revalidate,
                    collectedExpire: finalServerPrerenderStore.expire,
                    collectedStale: finalServerPrerenderStore.stale,
                    collectedTags: finalServerPrerenderStore.tags
                };
            }
        } else if (renderOpts.experimental.isRoutePPREnabled) {
            // We're statically generating with PPR and need to do dynamic tracking
            let dynamicTracking = (0, _dynamicrendering.createDynamicTrackingState)(renderOpts.isDebugDynamicAccesses);
            const prerenderResumeDataCache = (0, _resumedatacache.createPrerenderResumeDataCache)();
            const reactServerPrerenderStore = prerenderStore = {
                type: 'prerender-ppr',
                phase: 'render',
                implicitTags: implicitTags,
                dynamicTracking,
                revalidate: _constants1.INFINITE_CACHE,
                expire: _constants1.INFINITE_CACHE,
                stale: _constants1.INFINITE_CACHE,
                tags: [
                    ...implicitTags
                ],
                prerenderResumeDataCache
            };
            const RSCPayload = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(reactServerPrerenderStore, getRSCPayload, tree, ctx, res.statusCode === 404);
            const reactServerResult = reactServerPrerenderResult = await (0, _apprenderprerenderutils.createReactServerPrerenderResultFromRender)(_workunitasyncstorageexternal.workUnitAsyncStorage.run(reactServerPrerenderStore, ComponentMod.renderToReadableStream, // ... the arguments for the function to run
            RSCPayload, clientReferenceManifest.clientModules, {
                onError: serverComponentsErrorHandler
            }));
            const ssrPrerenderStore = {
                type: 'prerender-ppr',
                phase: 'render',
                implicitTags: implicitTags,
                dynamicTracking,
                revalidate: _constants1.INFINITE_CACHE,
                expire: _constants1.INFINITE_CACHE,
                stale: _constants1.INFINITE_CACHE,
                tags: [
                    ...implicitTags
                ],
                prerenderResumeDataCache
            };
            const prerender = require('react-dom/static.edge').prerender;
            const { prelude, postponed } = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(ssrPrerenderStore, prerender, /*#__PURE__*/ (0, _jsxruntime.jsx)(App, {
                reactServerStream: reactServerResult.asUnclosingStream(),
                preinitScripts: preinitScripts,
                clientReferenceManifest: clientReferenceManifest,
                ServerInsertedHTMLProvider: ServerInsertedHTMLProvider,
                nonce: ctx.nonce
            }), {
                onError: htmlRendererErrorHandler,
                onHeaders: (headers)=>{
                    headers.forEach((value, key)=>{
                        setHeader(key, value);
                    });
                },
                maxHeadersLength: renderOpts.reactMaxHeadersLength,
                // When debugging the static shell, client-side rendering should be
                // disabled to prevent blanking out the page.
                bootstrapScripts: renderOpts.isDebugStaticShell ? [] : [
                    bootstrapScript
                ]
            });
            const getServerInsertedHTML = (0, _makegetserverinsertedhtml.makeGetServerInsertedHTML)({
                polyfills,
                renderServerInsertedHTML,
                serverCapturedErrors: allCapturedErrors,
                basePath: renderOpts.basePath,
                tracingMetadata: tracingMetadata
            });
            // After awaiting here we've waited for the entire RSC render to complete. Crucially this means
            // that when we detect whether we've used dynamic APIs below we know we'll have picked up even
            // parts of the React Server render that might not be used in the SSR render.
            const flightData = await (0, _nodewebstreamshelper.streamToBuffer)(reactServerResult.asStream());
            if (shouldGenerateStaticFlightData(workStore)) {
                metadata.flightData = flightData;
                metadata.segmentData = await collectSegmentData(flightData, ssrPrerenderStore, ComponentMod, renderOpts);
            }
            /**
       * When prerendering there are three outcomes to consider
       *
       *   Dynamic HTML:      The prerender has dynamic holes (caused by using Next.js Dynamic Rendering APIs)
       *                      We will need to resume this result when requests are handled and we don't include
       *                      any server inserted HTML or inlined flight data in the static HTML
       *
       *   Dynamic Data:      The prerender has no dynamic holes but dynamic APIs were used. We will not
       *                      resume this render when requests are handled but we will generate new inlined
       *                      flight data since it is dynamic and differences may end up reconciling on the client
       *
       *   Static:            The prerender has no dynamic holes and no dynamic APIs were used. We statically encode
       *                      all server inserted HTML and flight data
       */ // First we check if we have any dynamic holes in our HTML prerender
            if ((0, _dynamicrendering.accessedDynamicData)(dynamicTracking.dynamicAccesses)) {
                if (postponed != null) {
                    // Dynamic HTML case.
                    metadata.postponed = await (0, _postponedstate.getDynamicHTMLPostponedState)(postponed, fallbackRouteParams, prerenderResumeDataCache);
                } else {
                    // Dynamic Data case.
                    metadata.postponed = await (0, _postponedstate.getDynamicDataPostponedState)(prerenderResumeDataCache);
                }
                // Regardless of whether this is the Dynamic HTML or Dynamic Data case we need to ensure we include
                // server inserted html in the static response because the html that is part of the prerender may depend on it
                // It is possible in the set of stream transforms for Dynamic HTML vs Dynamic Data may differ but currently both states
                // require the same set so we unify the code path here
                reactServerResult.consume();
                return {
                    digestErrorsMap: reactServerErrorsByDigest,
                    ssrErrors: allCapturedErrors,
                    stream: await (0, _nodewebstreamshelper.continueDynamicPrerender)(prelude, {
                        getServerInsertedHTML
                    }),
                    dynamicAccess: dynamicTracking.dynamicAccesses,
                    // TODO: Should this include the SSR pass?
                    collectedRevalidate: reactServerPrerenderStore.revalidate,
                    collectedExpire: reactServerPrerenderStore.expire,
                    collectedStale: reactServerPrerenderStore.stale,
                    collectedTags: reactServerPrerenderStore.tags
                };
            } else if (fallbackRouteParams && fallbackRouteParams.size > 0) {
                // Rendering the fallback case.
                metadata.postponed = await (0, _postponedstate.getDynamicDataPostponedState)(prerenderResumeDataCache);
                return {
                    digestErrorsMap: reactServerErrorsByDigest,
                    ssrErrors: allCapturedErrors,
                    stream: await (0, _nodewebstreamshelper.continueDynamicPrerender)(prelude, {
                        getServerInsertedHTML
                    }),
                    dynamicAccess: dynamicTracking.dynamicAccesses,
                    // TODO: Should this include the SSR pass?
                    collectedRevalidate: reactServerPrerenderStore.revalidate,
                    collectedExpire: reactServerPrerenderStore.expire,
                    collectedStale: reactServerPrerenderStore.stale,
                    collectedTags: reactServerPrerenderStore.tags
                };
            } else {
                // Static case
                // We still have not used any dynamic APIs. At this point we can produce an entirely static prerender response
                if (workStore.forceDynamic) {
                    throw new _staticgenerationbailout.StaticGenBailoutError('Invariant: a Page with `dynamic = "force-dynamic"` did not trigger the dynamic pathway. This is a bug in Next.js');
                }
                let htmlStream = prelude;
                if (postponed != null) {
                    // We postponed but nothing dynamic was used. We resume the render now and immediately abort it
                    // so we can set all the postponed boundaries to client render mode before we store the HTML response
                    const resume = require('react-dom/server.edge').resume;
                    // We don't actually want to render anything so we just pass a stream
                    // that never resolves. The resume call is going to abort immediately anyway
                    const foreverStream = new ReadableStream();
                    const resumeStream = await resume(/*#__PURE__*/ (0, _jsxruntime.jsx)(App, {
                        reactServerStream: foreverStream,
                        preinitScripts: ()=>{},
                        clientReferenceManifest: clientReferenceManifest,
                        ServerInsertedHTMLProvider: ServerInsertedHTMLProvider,
                        nonce: ctx.nonce
                    }), JSON.parse(JSON.stringify(postponed)), {
                        signal: (0, _dynamicrendering.createPostponedAbortSignal)('static prerender resume'),
                        onError: htmlRendererErrorHandler,
                        nonce: ctx.nonce
                    });
                    // First we write everything from the prerender, then we write everything from the aborted resume render
                    htmlStream = (0, _nodewebstreamshelper.chainStreams)(prelude, resumeStream);
                }
                return {
                    digestErrorsMap: reactServerErrorsByDigest,
                    ssrErrors: allCapturedErrors,
                    stream: await (0, _nodewebstreamshelper.continueStaticPrerender)(htmlStream, {
                        inlinedDataStream: (0, _useflightresponse.createInlinedDataReadableStream)(reactServerResult.consumeAsStream(), ctx.nonce, formState),
                        getServerInsertedHTML
                    }),
                    dynamicAccess: dynamicTracking.dynamicAccesses,
                    // TODO: Should this include the SSR pass?
                    collectedRevalidate: reactServerPrerenderStore.revalidate,
                    collectedExpire: reactServerPrerenderStore.expire,
                    collectedStale: reactServerPrerenderStore.stale,
                    collectedTags: reactServerPrerenderStore.tags
                };
            }
        } else {
            const prerenderLegacyStore = prerenderStore = {
                type: 'prerender-legacy',
                phase: 'render',
                implicitTags: implicitTags,
                revalidate: _constants1.INFINITE_CACHE,
                expire: _constants1.INFINITE_CACHE,
                stale: _constants1.INFINITE_CACHE,
                tags: [
                    ...implicitTags
                ]
            };
            // This is a regular static generation. We don't do dynamic tracking because we rely on
            // the old-school dynamic error handling to bail out of static generation
            const RSCPayload = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(prerenderLegacyStore, getRSCPayload, tree, ctx, res.statusCode === 404);
            const reactServerResult = reactServerPrerenderResult = await (0, _apprenderprerenderutils.createReactServerPrerenderResultFromRender)(_workunitasyncstorageexternal.workUnitAsyncStorage.run(prerenderLegacyStore, ComponentMod.renderToReadableStream, RSCPayload, clientReferenceManifest.clientModules, {
                onError: serverComponentsErrorHandler
            }));
            const renderToReadableStream = require('react-dom/server.edge').renderToReadableStream;
            const htmlStream = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(prerenderLegacyStore, renderToReadableStream, /*#__PURE__*/ (0, _jsxruntime.jsx)(App, {
                reactServerStream: reactServerResult.asUnclosingStream(),
                preinitScripts: preinitScripts,
                clientReferenceManifest: clientReferenceManifest,
                ServerInsertedHTMLProvider: ServerInsertedHTMLProvider,
                nonce: ctx.nonce
            }), {
                onError: htmlRendererErrorHandler,
                nonce: ctx.nonce,
                // When debugging the static shell, client-side rendering should be
                // disabled to prevent blanking out the page.
                bootstrapScripts: renderOpts.isDebugStaticShell ? [] : [
                    bootstrapScript
                ]
            });
            if (shouldGenerateStaticFlightData(workStore)) {
                const flightData = await (0, _nodewebstreamshelper.streamToBuffer)(reactServerResult.asStream());
                metadata.flightData = flightData;
                metadata.segmentData = await collectSegmentData(flightData, prerenderLegacyStore, ComponentMod, renderOpts);
            }
            const getServerInsertedHTML = (0, _makegetserverinsertedhtml.makeGetServerInsertedHTML)({
                polyfills,
                renderServerInsertedHTML,
                serverCapturedErrors: allCapturedErrors,
                basePath: renderOpts.basePath,
                tracingMetadata: tracingMetadata
            });
            return {
                digestErrorsMap: reactServerErrorsByDigest,
                ssrErrors: allCapturedErrors,
                stream: await (0, _nodewebstreamshelper.continueFizzStream)(htmlStream, {
                    inlinedDataStream: (0, _useflightresponse.createInlinedDataReadableStream)(reactServerResult.consumeAsStream(), ctx.nonce, formState),
                    isStaticGeneration: true,
                    getServerInsertedHTML,
                    serverInsertedHTMLToHead: true
                }),
                // TODO: Should this include the SSR pass?
                collectedRevalidate: prerenderLegacyStore.revalidate,
                collectedExpire: prerenderLegacyStore.expire,
                collectedStale: prerenderLegacyStore.stale,
                collectedTags: prerenderLegacyStore.tags
            };
        }
    } catch (err) {
        if ((0, _staticgenerationbailout.isStaticGenBailoutError)(err) || typeof err === 'object' && err !== null && 'message' in err && typeof err.message === 'string' && err.message.includes('https://nextjs.org/docs/advanced-features/static-html-export')) {
            // Ensure that "next dev" prints the red error overlay
            throw err;
        }
        // If this is a static generation error, we need to throw it so that it
        // can be handled by the caller if we're in static generation mode.
        if ((0, _hooksservercontext.isDynamicServerError)(err)) {
            throw err;
        }
        // If a bailout made it to this point, it means it wasn't wrapped inside
        // a suspense boundary.
        const shouldBailoutToCSR = (0, _bailouttocsr.isBailoutToCSRError)(err);
        if (shouldBailoutToCSR) {
            const stack = (0, _formatservererror.getStackWithoutErrorMessage)(err);
            (0, _log.error)(`${err.reason} should be wrapped in a suspense boundary at page "${ctx.pagePath}". Read more: https://nextjs.org/docs/messages/missing-suspense-with-csr-bailout\n${stack}`);
            throw err;
        }
        // If we errored when we did not have an RSC stream to read from. This is
        // not just a render error, we need to throw early.
        if (reactServerPrerenderResult === null) {
            throw err;
        }
        let errorType;
        if ((0, _httpaccessfallback.isHTTPAccessFallbackError)(err)) {
            res.statusCode = (0, _httpaccessfallback.getAccessFallbackHTTPStatus)(err);
            errorType = (0, _httpaccessfallback.getAccessFallbackErrorTypeByStatus)(res.statusCode);
        } else if ((0, _redirecterror.isRedirectError)(err)) {
            errorType = 'redirect';
            res.statusCode = (0, _redirect.getRedirectStatusCodeFromError)(err);
            const redirectUrl = (0, _addpathprefix.addPathPrefix)((0, _redirect.getURLFromRedirectError)(err), renderOpts.basePath);
            setHeader('location', redirectUrl);
        } else if (!shouldBailoutToCSR) {
            res.statusCode = 500;
        }
        const [errorPreinitScripts, errorBootstrapScript] = (0, _requiredscripts.getRequiredScripts)(renderOpts.buildManifest, ctx.assetPrefix, renderOpts.crossOrigin, renderOpts.subresourceIntegrityManifest, (0, _getassetquerystring.getAssetQueryString)(ctx, false), ctx.nonce, '/_not-found/page');
        const prerenderLegacyStore = prerenderStore = {
            type: 'prerender-legacy',
            phase: 'render',
            implicitTags: implicitTags,
            revalidate: _constants1.INFINITE_CACHE,
            expire: _constants1.INFINITE_CACHE,
            stale: _constants1.INFINITE_CACHE,
            tags: [
                ...implicitTags
            ]
        };
        const errorRSCPayload = await _workunitasyncstorageexternal.workUnitAsyncStorage.run(prerenderLegacyStore, getErrorRSCPayload, tree, ctx, errorType);
        const errorServerStream = _workunitasyncstorageexternal.workUnitAsyncStorage.run(prerenderLegacyStore, ComponentMod.renderToReadableStream, errorRSCPayload, clientReferenceManifest.clientModules, {
            onError: serverComponentsErrorHandler
        });
        try {
            const fizzStream = await (0, _nodewebstreamshelper.renderToInitialFizzStream)({
                ReactDOMServer: require('react-dom/server.edge'),
                element: /*#__PURE__*/ (0, _jsxruntime.jsx)(AppWithoutContext, {
                    reactServerStream: errorServerStream,
                    preinitScripts: errorPreinitScripts,
                    clientReferenceManifest: clientReferenceManifest,
                    nonce: ctx.nonce
                }),
                streamOptions: {
                    nonce: ctx.nonce,
                    // Include hydration scripts in the HTML
                    bootstrapScripts: [
                        errorBootstrapScript
                    ],
                    formState
                }
            });
            if (shouldGenerateStaticFlightData(workStore)) {
                const flightData = await (0, _nodewebstreamshelper.streamToBuffer)(reactServerPrerenderResult.asStream());
                metadata.flightData = flightData;
                metadata.segmentData = await collectSegmentData(flightData, prerenderLegacyStore, ComponentMod, renderOpts);
            }
            const validateRootLayout = renderOpts.dev;
            // This is intentionally using the readable datastream from the main
            // render rather than the flight data from the error page render
            const flightStream = reactServerPrerenderResult instanceof _apprenderprerenderutils.ServerPrerenderStreamResult ? reactServerPrerenderResult.asStream() : reactServerPrerenderResult.consumeAsStream();
            return {
                // Returning the error that was thrown so it can be used to handle
                // the response in the caller.
                digestErrorsMap: reactServerErrorsByDigest,
                ssrErrors: allCapturedErrors,
                stream: await (0, _nodewebstreamshelper.continueFizzStream)(fizzStream, {
                    inlinedDataStream: (0, _useflightresponse.createInlinedDataReadableStream)(flightStream, ctx.nonce, formState),
                    isStaticGeneration: true,
                    getServerInsertedHTML: (0, _makegetserverinsertedhtml.makeGetServerInsertedHTML)({
                        polyfills,
                        renderServerInsertedHTML,
                        serverCapturedErrors: [],
                        basePath: renderOpts.basePath,
                        tracingMetadata: tracingMetadata
                    }),
                    serverInsertedHTMLToHead: true,
                    validateRootLayout
                }),
                dynamicAccess: null,
                collectedRevalidate: prerenderStore !== null ? prerenderStore.revalidate : _constants1.INFINITE_CACHE,
                collectedExpire: prerenderStore !== null ? prerenderStore.expire : _constants1.INFINITE_CACHE,
                collectedStale: prerenderStore !== null ? prerenderStore.stale : _constants1.INFINITE_CACHE,
                collectedTags: prerenderStore !== null ? prerenderStore.tags : null
            };
        } catch (finalErr) {
            if (process.env.NODE_ENV === 'development' && (0, _httpaccessfallback.isHTTPAccessFallbackError)(finalErr)) {
                const { bailOnRootNotFound } = require('../../client/components/dev-root-http-access-fallback-boundary');
                bailOnRootNotFound();
            }
            throw finalErr;
        }
    }
}
const loadingChunks = new Set();
const chunkListeners = [];
function trackChunkLoading(load) {
    loadingChunks.add(load);
    load.finally(()=>{
        if (loadingChunks.has(load)) {
            loadingChunks.delete(load);
            if (loadingChunks.size === 0) {
                // We are not currently loading any chunks. We can notify all listeners
                for(let i = 0; i < chunkListeners.length; i++){
                    chunkListeners[i]();
                }
                chunkListeners.length = 0;
            }
        }
    });
}
async function warmFlightResponse(flightStream, clientReferenceManifest) {
    let createFromReadableStream;
    if (process.env.TURBOPACK) {
        createFromReadableStream = // eslint-disable-next-line import/no-extraneous-dependencies
        require('react-server-dom-turbopack/client.edge').createFromReadableStream;
    } else {
        createFromReadableStream = // eslint-disable-next-line import/no-extraneous-dependencies
        require('react-server-dom-webpack/client.edge').createFromReadableStream;
    }
    try {
        createFromReadableStream(flightStream, {
            serverConsumerManifest: {
                moduleLoading: clientReferenceManifest.moduleLoading,
                moduleMap: clientReferenceManifest.ssrModuleMapping,
                serverModuleMap: null
            }
        });
    } catch  {
    // We don't want to handle errors here but we don't want it to
    // interrupt the outer flow. We simply ignore it here and expect
    // it will bubble up during a render
    }
    // We'll wait at least one task and then if no chunks have started to load
    // we'll we can infer that there are none to load from this flight response
    trackChunkLoading((0, _scheduler.waitAtLeastOneReactRenderTask)());
    return new Promise((r)=>{
        chunkListeners.push(r);
    });
}
const getGlobalErrorStyles = async (tree, ctx)=>{
    const { modules: { 'global-error': globalErrorModule } } = (0, _parseloadertree.parseLoaderTree)(tree);
    let globalErrorStyles;
    if (globalErrorModule) {
        const [, styles] = await (0, _createcomponentstylesandscripts.createComponentStylesAndScripts)({
            ctx,
            filePath: globalErrorModule[1],
            getComponent: globalErrorModule[0],
            injectedCSS: new Set(),
            injectedJS: new Set()
        });
        globalErrorStyles = styles;
    }
    return globalErrorStyles;
};
async function collectSegmentData(fullPageDataBuffer, prerenderStore, ComponentMod, renderOpts) {
    // Per-segment prefetch data
    //
    // All of the segments for a page are generated simultaneously, including
    // during revalidations. This is to ensure consistency, because it's
    // possible for a mismatch between a layout and page segment can cause the
    // client to error during rendering. We want to preserve the ability of the
    // client to recover from such a mismatch by re-requesting all the segments
    // to get a consistent view of the page.
    //
    // For performance, we reuse the Flight output that was created when
    // generating the initial page HTML. The Flight stream for the whole page is
    // decomposed into a separate stream per segment.
    const clientReferenceManifest = renderOpts.clientReferenceManifest;
    if (!clientReferenceManifest || renderOpts.experimental.isRoutePPREnabled !== true) {
        return;
    }
    // Manifest passed to the Flight client for reading the full-page Flight
    // stream. Based off similar code in use-cache-wrapper.ts.
    const isEdgeRuntime = process.env.NEXT_RUNTIME === 'edge';
    const serverConsumerManifest = {
        // moduleLoading must be null because we don't want to trigger preloads of ClientReferences
        // to be added to the consumer. Instead, we'll wait for any ClientReference to be emitted
        // which themselves will handle the preloading.
        moduleLoading: null,
        moduleMap: isEdgeRuntime ? clientReferenceManifest.edgeRscModuleMapping : clientReferenceManifest.rscModuleMapping,
        serverModuleMap: null
    };
    const staleTime = prerenderStore.stale;
    return await ComponentMod.collectSegmentData(fullPageDataBuffer, staleTime, clientReferenceManifest.clientModules, serverConsumerManifest);
}

//# sourceMappingURL=app-render.js.map