"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "createComponentTree", {
    enumerable: true,
    get: function() {
        return createComponentTree;
    }
});
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_default(require("react"));
const _clientreference = require("../../lib/client-reference");
const _appdirmodule = require("../lib/app-dir-module");
const _interopdefault = require("./interop-default");
const _parseloadertree = require("./parse-loader-tree");
const _createcomponentstylesandscripts = require("./create-component-styles-and-scripts");
const _getlayerassets = require("./get-layer-assets");
const _hasloadingcomponentintree = require("./has-loading-component-in-tree");
const _patchfetch = require("../lib/patch-fetch");
const _parallelroutedefault = require("../../client/components/parallel-route-default");
const _tracer = require("../lib/trace/tracer");
const _constants = require("../lib/trace/constants");
const _staticgenerationbailout = require("../../client/components/static-generation-bailout");
const _workunitasyncstorageexternal = require("./work-unit-async-storage.external");
const _metadataconstants = require("../../lib/metadata/metadata-constants");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function createComponentTree(props) {
    return (0, _tracer.getTracer)().trace(_constants.NextNodeServerSpan.createComponentTree, {
        spanName: 'build component tree'
    }, ()=>createComponentTreeInternal(props));
}
function errorMissingDefaultExport(pagePath, convention) {
    const normalizedPagePath = pagePath === '/' ? '' : pagePath;
    throw new Error(`The default export is not a React Component in "${normalizedPagePath}/${convention}"`);
}
const cacheNodeKey = 'c';
async function createComponentTreeInternal({ createSegmentPath, loaderTree: tree, parentParams, firstItem, rootLayoutIncluded, injectedCSS, injectedJS, injectedFontPreloadTags, getMetadataReady, ctx, missingSlots, preloadCallbacks, authInterrupts }) {
    const { renderOpts: { nextConfigOutput, experimental }, workStore, componentMod: { HTTPAccessFallbackBoundary, LayoutRouter, RenderFromTemplateContext, OutletBoundary, ClientPageRoot, ClientSegmentRoot, createServerSearchParamsForServerPage, createPrerenderSearchParamsForClientPage, createServerParamsForServerSegment, createPrerenderParamsForClientSegment, serverHooks: { DynamicServerError }, Postpone }, pagePath, getDynamicParamFromSegment, isPrefetch, query } = ctx;
    const { page, layoutOrPagePath, segment, modules, parallelRoutes } = (0, _parseloadertree.parseLoaderTree)(tree);
    const { layout, template, error, loading, 'not-found': notFound, forbidden, unauthorized } = modules;
    const injectedCSSWithCurrentLayout = new Set(injectedCSS);
    const injectedJSWithCurrentLayout = new Set(injectedJS);
    const injectedFontPreloadTagsWithCurrentLayout = new Set(injectedFontPreloadTags);
    const layerAssets = (0, _getlayerassets.getLayerAssets)({
        preloadCallbacks,
        ctx,
        layoutOrPagePath,
        injectedCSS: injectedCSSWithCurrentLayout,
        injectedJS: injectedJSWithCurrentLayout,
        injectedFontPreloadTags: injectedFontPreloadTagsWithCurrentLayout
    });
    const [Template, templateStyles, templateScripts] = template ? await (0, _createcomponentstylesandscripts.createComponentStylesAndScripts)({
        ctx,
        filePath: template[1],
        getComponent: template[0],
        injectedCSS: injectedCSSWithCurrentLayout,
        injectedJS: injectedJSWithCurrentLayout
    }) : [
        _react.default.Fragment
    ];
    const [ErrorComponent, errorStyles, errorScripts] = error ? await (0, _createcomponentstylesandscripts.createComponentStylesAndScripts)({
        ctx,
        filePath: error[1],
        getComponent: error[0],
        injectedCSS: injectedCSSWithCurrentLayout,
        injectedJS: injectedJSWithCurrentLayout
    }) : [];
    const [Loading, loadingStyles, loadingScripts] = loading ? await (0, _createcomponentstylesandscripts.createComponentStylesAndScripts)({
        ctx,
        filePath: loading[1],
        getComponent: loading[0],
        injectedCSS: injectedCSSWithCurrentLayout,
        injectedJS: injectedJSWithCurrentLayout
    }) : [];
    const isLayout = typeof layout !== 'undefined';
    const isPage = typeof page !== 'undefined';
    const { mod: layoutOrPageMod } = await (0, _tracer.getTracer)().trace(_constants.NextNodeServerSpan.getLayoutOrPageModule, {
        hideSpan: !(isLayout || isPage),
        spanName: 'resolve segment modules',
        attributes: {
            'next.segment': segment
        }
    }, ()=>(0, _appdirmodule.getLayoutOrPageModule)(tree));
    /**
   * Checks if the current segment is a root layout.
   */ const rootLayoutAtThisLevel = isLayout && !rootLayoutIncluded;
    /**
   * Checks if the current segment or any level above it has a root layout.
   */ const rootLayoutIncludedAtThisLevelOrAbove = rootLayoutIncluded || rootLayoutAtThisLevel;
    const [NotFound, notFoundStyles] = notFound ? await (0, _createcomponentstylesandscripts.createComponentStylesAndScripts)({
        ctx,
        filePath: notFound[1],
        getComponent: notFound[0],
        injectedCSS: injectedCSSWithCurrentLayout,
        injectedJS: injectedJSWithCurrentLayout
    }) : [];
    const [Forbidden, forbiddenStyles] = authInterrupts && forbidden ? await (0, _createcomponentstylesandscripts.createComponentStylesAndScripts)({
        ctx,
        filePath: forbidden[1],
        getComponent: forbidden[0],
        injectedCSS: injectedCSSWithCurrentLayout,
        injectedJS: injectedJSWithCurrentLayout
    }) : [];
    const forbiddenElement = Forbidden ? /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
        children: [
            forbiddenStyles,
            /*#__PURE__*/ (0, _jsxruntime.jsx)(Forbidden, {})
        ]
    }) : undefined;
    const [Unauthorized, unauthorizedStyles] = authInterrupts && unauthorized ? await (0, _createcomponentstylesandscripts.createComponentStylesAndScripts)({
        ctx,
        filePath: unauthorized[1],
        getComponent: unauthorized[0],
        injectedCSS: injectedCSSWithCurrentLayout,
        injectedJS: injectedJSWithCurrentLayout
    }) : [];
    const unauthorizedElement = Unauthorized ? /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
        children: [
            unauthorizedStyles,
            /*#__PURE__*/ (0, _jsxruntime.jsx)(Unauthorized, {})
        ]
    }) : undefined;
    let dynamic = layoutOrPageMod == null ? void 0 : layoutOrPageMod.dynamic;
    if (nextConfigOutput === 'export') {
        if (!dynamic || dynamic === 'auto') {
            dynamic = 'error';
        } else if (dynamic === 'force-dynamic') {
            // force-dynamic is always incompatible with 'export'. We must interrupt the build
            throw new _staticgenerationbailout.StaticGenBailoutError(`Page with \`dynamic = "force-dynamic"\` couldn't be exported. \`output: "export"\` requires all pages be renderable statically because there is not runtime server to dynamic render routes in this output format. Learn more: https://nextjs.org/docs/app/building-your-application/deploying/static-exports`);
        }
    }
    if (typeof dynamic === 'string') {
        // the nested most config wins so we only force-static
        // if it's configured above any parent that configured
        // otherwise
        if (dynamic === 'error') {
            workStore.dynamicShouldError = true;
        } else if (dynamic === 'force-dynamic') {
            workStore.forceDynamic = true;
            // TODO: (PPR) remove this bailout once PPR is the default
            if (workStore.isStaticGeneration && !experimental.isRoutePPREnabled) {
                // If the postpone API isn't available, we can't postpone the render and
                // therefore we can't use the dynamic API.
                const err = new DynamicServerError(`Page with \`dynamic = "force-dynamic"\` won't be rendered statically.`);
                workStore.dynamicUsageDescription = err.message;
                workStore.dynamicUsageStack = err.stack;
                throw err;
            }
        } else {
            workStore.dynamicShouldError = false;
            workStore.forceStatic = dynamic === 'force-static';
        }
    }
    if (typeof (layoutOrPageMod == null ? void 0 : layoutOrPageMod.fetchCache) === 'string') {
        workStore.fetchCache = layoutOrPageMod == null ? void 0 : layoutOrPageMod.fetchCache;
    }
    if (typeof (layoutOrPageMod == null ? void 0 : layoutOrPageMod.revalidate) !== 'undefined') {
        (0, _patchfetch.validateRevalidate)(layoutOrPageMod == null ? void 0 : layoutOrPageMod.revalidate, workStore.route);
    }
    if (typeof (layoutOrPageMod == null ? void 0 : layoutOrPageMod.revalidate) === 'number') {
        const defaultRevalidate = layoutOrPageMod.revalidate;
        const workUnitStore = _workunitasyncstorageexternal.workUnitAsyncStorage.getStore();
        if (workUnitStore) {
            if (workUnitStore.type === 'prerender' || workUnitStore.type === 'prerender-legacy' || workUnitStore.type === 'prerender-ppr' || workUnitStore.type === 'cache') {
                if (workUnitStore.revalidate > defaultRevalidate) {
                    workUnitStore.revalidate = defaultRevalidate;
                }
            }
        }
        if (!workStore.forceStatic && workStore.isStaticGeneration && defaultRevalidate === 0 && // If the postpone API isn't available, we can't postpone the render and
        // therefore we can't use the dynamic API.
        !experimental.isRoutePPREnabled) {
            const dynamicUsageDescription = `revalidate: 0 configured ${segment}`;
            workStore.dynamicUsageDescription = dynamicUsageDescription;
            throw new DynamicServerError(dynamicUsageDescription);
        }
    }
    const isStaticGeneration = workStore.isStaticGeneration;
    // Assume the segment we're rendering contains only partial data if PPR is
    // enabled and this is a statically generated response. This is used by the
    // client Segment Cache after a prefetch to determine if it can skip the
    // second request to fill in the dynamic data.
    //
    // It's OK for this to be `true` when the data is actually fully static, but
    // it's not OK for this to be `false` when the data possibly contains holes.
    // Although the value here is overly pessimistic, for prefetches, it will be
    // replaced by a more specific value when the data is later processed into
    // per-segment responses (see collect-segment-data.tsx)
    //
    // For dynamic requests, this must always be `false` because dynamic responses
    // are never partial.
    const isPossiblyPartialResponse = isStaticGeneration && experimental.isRoutePPREnabled === true;
    // If there's a dynamic usage error attached to the store, throw it.
    if (workStore.dynamicUsageErr) {
        throw workStore.dynamicUsageErr;
    }
    const LayoutOrPage = layoutOrPageMod ? (0, _interopdefault.interopDefault)(layoutOrPageMod) : undefined;
    /**
   * The React Component to render.
   */ let MaybeComponent = LayoutOrPage;
    if (process.env.NODE_ENV === 'development') {
        const { isValidElementType } = require('next/dist/compiled/react-is');
        if ((isPage || typeof MaybeComponent !== 'undefined') && !isValidElementType(MaybeComponent)) {
            errorMissingDefaultExport(pagePath, 'page');
        }
        if (typeof ErrorComponent !== 'undefined' && !isValidElementType(ErrorComponent)) {
            errorMissingDefaultExport(pagePath, 'error');
        }
        if (typeof Loading !== 'undefined' && !isValidElementType(Loading)) {
            errorMissingDefaultExport(pagePath, 'loading');
        }
        if (typeof NotFound !== 'undefined' && !isValidElementType(NotFound)) {
            errorMissingDefaultExport(pagePath, 'not-found');
        }
        if (typeof Forbidden !== 'undefined' && !isValidElementType(Forbidden)) {
            errorMissingDefaultExport(pagePath, 'forbidden');
        }
        if (typeof Unauthorized !== 'undefined' && !isValidElementType(Unauthorized)) {
            errorMissingDefaultExport(pagePath, 'unauthorized');
        }
    }
    // Handle dynamic segment params.
    const segmentParam = getDynamicParamFromSegment(segment);
    // Create object holding the parent params and current params
    let currentParams = parentParams;
    if (segmentParam && segmentParam.value !== null) {
        currentParams = {
            ...parentParams,
            [segmentParam.param]: segmentParam.value
        };
    }
    // Resolve the segment param
    const actualSegment = segmentParam ? segmentParam.treeSegment : segment;
    //
    // TODO: Combine this `map` traversal with the loop below that turns the array
    // into an object.
    const parallelRouteMap = await Promise.all(Object.keys(parallelRoutes).map(async (parallelRouteKey)=>{
        const isChildrenRouteKey = parallelRouteKey === 'children';
        const currentSegmentPath = firstItem ? [
            parallelRouteKey
        ] : [
            actualSegment,
            parallelRouteKey
        ];
        const parallelRoute = parallelRoutes[parallelRouteKey];
        const notFoundComponent = NotFound && isChildrenRouteKey ? /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
            children: [
                notFoundStyles,
                /*#__PURE__*/ (0, _jsxruntime.jsx)(NotFound, {})
            ]
        }) : undefined;
        const forbiddenComponent = isChildrenRouteKey ? forbiddenElement : undefined;
        const unauthorizedComponent = isChildrenRouteKey ? unauthorizedElement : undefined;
        // if we're prefetching and that there's a Loading component, we bail out
        // otherwise we keep rendering for the prefetch.
        // We also want to bail out if there's no Loading component in the tree.
        let childCacheNodeSeedData = null;
        if (// Before PPR, the way instant navigations work in Next.js is we
        // prefetch everything up to the first route segment that defines a
        // loading.tsx boundary. (We do the same if there's no loading
        // boundary in the entire tree, because we don't want to prefetch too
        // much) The rest of the tree is deferred until the actual navigation.
        // It does not take into account whether the data is dynamic — even if
        // the tree is completely static, it will still defer everything
        // inside the loading boundary.
        //
        // This behavior predates PPR and is only relevant if the
        // PPR flag is not enabled.
        isPrefetch && (Loading || !(0, _hasloadingcomponentintree.hasLoadingComponentInTree)(parallelRoute)) && // The approach with PPR is different — loading.tsx behaves like a
        // regular Suspense boundary and has no special behavior.
        //
        // With PPR, we prefetch as deeply as possible, and only defer when
        // dynamic data is accessed. If so, we only defer the nearest parent
        // Suspense boundary of the dynamic data access, regardless of whether
        // the boundary is defined by loading.tsx or a normal <Suspense>
        // component in userspace.
        //
        // NOTE: In practice this usually means we'll end up prefetching more
        // than we were before PPR, which may or may not be considered a
        // performance regression by some apps. The plan is to address this
        // before General Availability of PPR by introducing granular
        // per-segment fetching, so we can reuse as much of the tree as
        // possible during both prefetches and dynamic navigations. But during
        // the beta period, we should be clear about this trade off in our
        // communications.
        !experimental.isRoutePPREnabled) {
        // Don't prefetch this child. This will trigger a lazy fetch by the
        // client router.
        } else {
            // Create the child component
            if (process.env.NODE_ENV === 'development' && missingSlots) {
                var _parsedTree_layoutOrPagePath;
                // When we detect the default fallback (which triggers a 404), we collect the missing slots
                // to provide more helpful debug information during development mode.
                const parsedTree = (0, _parseloadertree.parseLoaderTree)(parallelRoute);
                if ((_parsedTree_layoutOrPagePath = parsedTree.layoutOrPagePath) == null ? void 0 : _parsedTree_layoutOrPagePath.endsWith(_parallelroutedefault.PARALLEL_ROUTE_DEFAULT_PATH)) {
                    missingSlots.add(parallelRouteKey);
                }
            }
            const seedData = await createComponentTreeInternal({
                createSegmentPath: (child)=>{
                    return createSegmentPath([
                        ...currentSegmentPath,
                        ...child
                    ]);
                },
                loaderTree: parallelRoute,
                parentParams: currentParams,
                rootLayoutIncluded: rootLayoutIncludedAtThisLevelOrAbove,
                injectedCSS: injectedCSSWithCurrentLayout,
                injectedJS: injectedJSWithCurrentLayout,
                injectedFontPreloadTags: injectedFontPreloadTagsWithCurrentLayout,
                // getMetadataReady is used to conditionally throw. In the case of parallel routes we will have more than one page
                // but we only want to throw on the first one.
                getMetadataReady: isChildrenRouteKey ? getMetadataReady : ()=>Promise.resolve(),
                ctx,
                missingSlots,
                preloadCallbacks,
                authInterrupts: authInterrupts
            });
            childCacheNodeSeedData = seedData;
        }
        // This is turned back into an object below.
        return [
            parallelRouteKey,
            /*#__PURE__*/ (0, _jsxruntime.jsx)(LayoutRouter, {
                parallelRouterKey: parallelRouteKey,
                segmentPath: createSegmentPath(currentSegmentPath),
                // TODO-APP: Add test for loading returning `undefined`. This currently can't be tested as the `webdriver()` tab will wait for the full page to load before returning.
                error: ErrorComponent,
                errorStyles: errorStyles,
                errorScripts: errorScripts,
                template: /*#__PURE__*/ (0, _jsxruntime.jsx)(Template, {
                    children: /*#__PURE__*/ (0, _jsxruntime.jsx)(RenderFromTemplateContext, {})
                }),
                templateStyles: templateStyles,
                templateScripts: templateScripts,
                notFound: notFoundComponent,
                forbidden: forbiddenComponent,
                unauthorized: unauthorizedComponent
            }),
            childCacheNodeSeedData
        ];
    }));
    // Convert the parallel route map into an object after all promises have been resolved.
    let parallelRouteProps = {};
    let parallelRouteCacheNodeSeedData = {};
    for (const parallelRoute of parallelRouteMap){
        const [parallelRouteKey, parallelRouteProp, flightData] = parallelRoute;
        parallelRouteProps[parallelRouteKey] = parallelRouteProp;
        parallelRouteCacheNodeSeedData[parallelRouteKey] = flightData;
    }
    const loadingData = Loading ? [
        /*#__PURE__*/ (0, _jsxruntime.jsx)(Loading, {}, "l"),
        loadingStyles,
        loadingScripts
    ] : null;
    // When the segment does not have a layout or page we still have to add the layout router to ensure the path holds the loading component
    if (!MaybeComponent) {
        return [
            actualSegment,
            /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.default.Fragment, {
                children: [
                    layerAssets,
                    parallelRouteProps.children
                ]
            }, cacheNodeKey),
            parallelRouteCacheNodeSeedData,
            loadingData,
            isPossiblyPartialResponse
        ];
    }
    const Component = MaybeComponent;
    // If force-dynamic is used and the current render supports postponing, we
    // replace it with a node that will postpone the render. This ensures that the
    // postpone is invoked during the react render phase and not during the next
    // render phase.
    // @TODO this does not actually do what it seems like it would or should do. The idea is that
    // if we are rendering in a force-dynamic mode and we can postpone we should only make the segments
    // that ask for force-dynamic to be dynamic, allowing other segments to still prerender. However
    // because this comes after the children traversal and the static generation store is mutated every segment
    // along the parent path of a force-dynamic segment will hit this condition effectively making the entire
    // render force-dynamic. We should refactor this function so that we can correctly track which segments
    // need to be dynamic
    if (workStore.isStaticGeneration && workStore.forceDynamic && experimental.isRoutePPREnabled) {
        return [
            actualSegment,
            /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.default.Fragment, {
                children: [
                    /*#__PURE__*/ (0, _jsxruntime.jsx)(Postpone, {
                        reason: 'dynamic = "force-dynamic" was used',
                        route: workStore.route
                    }),
                    layerAssets
                ]
            }, cacheNodeKey),
            parallelRouteCacheNodeSeedData,
            loadingData,
            true
        ];
    }
    const isClientComponent = (0, _clientreference.isClientReference)(layoutOrPageMod);
    if (process.env.NODE_ENV === 'development' && 'params' in parallelRouteProps) {
        // @TODO consider making this an error and running the check in build as well
        console.error(`"params" is a reserved prop in Layouts and Pages and cannot be used as the name of a parallel route in ${segment}`);
    }
    if (isPage) {
        const PageComponent = Component;
        // Assign searchParams to props if this is a page
        let pageElement;
        if (isClientComponent) {
            if (isStaticGeneration) {
                const promiseOfParams = createPrerenderParamsForClientSegment(currentParams, workStore);
                const promiseOfSearchParams = createPrerenderSearchParamsForClientPage(workStore);
                pageElement = /*#__PURE__*/ (0, _jsxruntime.jsx)(ClientPageRoot, {
                    Component: PageComponent,
                    searchParams: query,
                    params: currentParams,
                    promises: [
                        promiseOfSearchParams,
                        promiseOfParams
                    ]
                });
            } else {
                pageElement = /*#__PURE__*/ (0, _jsxruntime.jsx)(ClientPageRoot, {
                    Component: PageComponent,
                    searchParams: query,
                    params: currentParams
                });
            }
        } else {
            // If we are passing searchParams to a server component Page we need to track their usage in case
            // the current render mode tracks dynamic API usage.
            const params = createServerParamsForServerSegment(currentParams, workStore);
            const searchParams = createServerSearchParamsForServerPage(query, workStore);
            pageElement = /*#__PURE__*/ (0, _jsxruntime.jsx)(PageComponent, {
                params: params,
                searchParams: searchParams
            });
        }
        return [
            actualSegment,
            /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.default.Fragment, {
                children: [
                    pageElement,
                    layerAssets,
                    /*#__PURE__*/ (0, _jsxruntime.jsx)(OutletBoundary, {
                        children: /*#__PURE__*/ (0, _jsxruntime.jsx)(MetadataOutlet, {
                            ready: getMetadataReady
                        })
                    })
                ]
            }, cacheNodeKey),
            parallelRouteCacheNodeSeedData,
            loadingData,
            isPossiblyPartialResponse
        ];
    } else {
        const SegmentComponent = Component;
        const isRootLayoutWithChildrenSlotAndAtLeastOneMoreSlot = rootLayoutAtThisLevel && 'children' in parallelRoutes && Object.keys(parallelRoutes).length > 1;
        let segmentNode;
        if (isClientComponent) {
            let clientSegment;
            if (isStaticGeneration) {
                const promiseOfParams = createPrerenderParamsForClientSegment(currentParams, workStore);
                clientSegment = /*#__PURE__*/ (0, _jsxruntime.jsx)(ClientSegmentRoot, {
                    Component: SegmentComponent,
                    slots: parallelRouteProps,
                    params: currentParams,
                    promise: promiseOfParams
                });
            } else {
                clientSegment = /*#__PURE__*/ (0, _jsxruntime.jsx)(ClientSegmentRoot, {
                    Component: SegmentComponent,
                    slots: parallelRouteProps,
                    params: currentParams
                });
            }
            if (isRootLayoutWithChildrenSlotAndAtLeastOneMoreSlot) {
                let notfoundClientSegment;
                let forbiddenClientSegment;
                let unauthorizedClientSegment;
                // TODO-APP: This is a hack to support unmatched parallel routes, which will throw `notFound()`.
                // This ensures that a `HTTPAccessFallbackBoundary` is available for when that happens,
                // but it's not ideal, as it needlessly invokes the `NotFound` component and renders the `RootLayout` twice.
                // We should instead look into handling the fallback behavior differently in development mode so that it doesn't
                // rely on the `NotFound` behavior.
                if (NotFound) {
                    const notFoundParallelRouteProps = {
                        children: /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
                            children: [
                                notFoundStyles,
                                /*#__PURE__*/ (0, _jsxruntime.jsx)(NotFound, {})
                            ]
                        })
                    };
                    notfoundClientSegment = /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
                        children: [
                            layerAssets,
                            /*#__PURE__*/ (0, _jsxruntime.jsx)(ClientSegmentRoot, {
                                Component: SegmentComponent,
                                slots: notFoundParallelRouteProps,
                                params: currentParams
                            })
                        ]
                    });
                }
                if (Forbidden) {
                    const forbiddenParallelRouteProps = {
                        children: forbiddenElement
                    };
                    forbiddenClientSegment = /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
                        children: [
                            layerAssets,
                            /*#__PURE__*/ (0, _jsxruntime.jsx)(ClientSegmentRoot, {
                                Component: SegmentComponent,
                                slots: forbiddenParallelRouteProps,
                                params: currentParams
                            })
                        ]
                    });
                }
                if (Unauthorized) {
                    const unauthorizedParallelRouteProps = {
                        children: unauthorizedElement
                    };
                    unauthorizedClientSegment = /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
                        children: [
                            layerAssets,
                            /*#__PURE__*/ (0, _jsxruntime.jsx)(ClientSegmentRoot, {
                                Component: SegmentComponent,
                                slots: unauthorizedParallelRouteProps,
                                params: currentParams
                            })
                        ]
                    });
                }
                if (notfoundClientSegment || forbiddenClientSegment || unauthorizedClientSegment) {
                    segmentNode = /*#__PURE__*/ (0, _jsxruntime.jsxs)(HTTPAccessFallbackBoundary, {
                        notFound: notfoundClientSegment,
                        forbidden: forbiddenClientSegment,
                        unauthorized: unauthorizedClientSegment,
                        children: [
                            layerAssets,
                            clientSegment
                        ]
                    }, cacheNodeKey);
                } else {
                    segmentNode = /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.default.Fragment, {
                        children: [
                            layerAssets,
                            clientSegment
                        ]
                    }, cacheNodeKey);
                }
            } else {
                segmentNode = /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.default.Fragment, {
                    children: [
                        layerAssets,
                        clientSegment
                    ]
                }, cacheNodeKey);
            }
        } else {
            const params = createServerParamsForServerSegment(currentParams, workStore);
            let serverSegment = /*#__PURE__*/ (0, _jsxruntime.jsx)(SegmentComponent, {
                ...parallelRouteProps,
                params: params
            });
            if (isRootLayoutWithChildrenSlotAndAtLeastOneMoreSlot) {
                // TODO-APP: This is a hack to support unmatched parallel routes, which will throw `notFound()`.
                // This ensures that a `HTTPAccessFallbackBoundary` is available for when that happens,
                // but it's not ideal, as it needlessly invokes the `NotFound` component and renders the `RootLayout` twice.
                // We should instead look into handling the fallback behavior differently in development mode so that it doesn't
                // rely on the `NotFound` behavior.
                segmentNode = /*#__PURE__*/ (0, _jsxruntime.jsxs)(HTTPAccessFallbackBoundary, {
                    notFound: NotFound ? /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
                        children: [
                            layerAssets,
                            /*#__PURE__*/ (0, _jsxruntime.jsxs)(SegmentComponent, {
                                params: params,
                                children: [
                                    notFoundStyles,
                                    /*#__PURE__*/ (0, _jsxruntime.jsx)(NotFound, {})
                                ]
                            })
                        ]
                    }) : undefined,
                    children: [
                        layerAssets,
                        serverSegment
                    ]
                }, cacheNodeKey);
            } else {
                segmentNode = /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.default.Fragment, {
                    children: [
                        layerAssets,
                        serverSegment
                    ]
                }, cacheNodeKey);
            }
        }
        // For layouts we just render the component
        return [
            actualSegment,
            segmentNode,
            parallelRouteCacheNodeSeedData,
            loadingData,
            isPossiblyPartialResponse
        ];
    }
}
async function MetadataOutlet({ ready }) {
    const r = ready();
    // We can avoid a extra microtask by unwrapping the instrumented promise directly if available.
    if (r.status === 'rejected') {
        throw r.value;
    } else if (r.status !== 'fulfilled') {
        await r;
    }
    return null;
}
MetadataOutlet.displayName = _metadataconstants.OUTLET_BOUNDARY_NAME;

//# sourceMappingURL=create-component-tree.js.map