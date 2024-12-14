'use client';
import { jsx as _jsx, jsxs as _jsxs, Fragment as _Fragment } from "react/jsx-runtime";
import React, { use, useEffect, useMemo, useCallback, startTransition, useInsertionEffect, useDeferredValue } from 'react';
import { AppRouterContext, LayoutRouterContext, GlobalLayoutRouterContext } from '../../shared/lib/app-router-context.shared-runtime';
import { ACTION_HMR_REFRESH, ACTION_NAVIGATE, ACTION_PREFETCH, ACTION_REFRESH, ACTION_RESTORE, ACTION_SERVER_PATCH, PrefetchKind } from './router-reducer/router-reducer-types';
import { createHrefFromUrl } from './router-reducer/create-href-from-url';
import { SearchParamsContext, PathnameContext, PathParamsContext } from '../../shared/lib/hooks-client-context.shared-runtime';
import { useReducer, useUnwrapState } from './use-reducer';
import { ErrorBoundary } from './error-boundary';
import { isBot } from '../../shared/lib/router/utils/is-bot';
import { addBasePath } from '../add-base-path';
import { AppRouterAnnouncer } from './app-router-announcer';
import { RedirectBoundary } from './redirect-boundary';
import { findHeadInCache } from './router-reducer/reducers/find-head-in-cache';
import { unresolvedThenable } from './unresolved-thenable';
import { removeBasePath } from '../remove-base-path';
import { hasBasePath } from '../has-base-path';
import { getSelectedParams } from './router-reducer/compute-changed-path';
import { useNavFailureHandler } from './nav-failure-handler';
import { useServerActionDispatcher } from '../app-call-server';
import { prefetch as prefetchWithSegmentCache } from '../components/segment-cache/prefetch';
import { getRedirectTypeFromError, getURLFromRedirectError } from './redirect';
import { isRedirectError, RedirectType } from './redirect-error';
const globalMutable = {};
function isExternalURL(url) {
    return url.origin !== window.location.origin;
}
/**
 * Given a link href, constructs the URL that should be prefetched. Returns null
 * in cases where prefetching should be disabled, like external URLs, or
 * during development.
 * @param href The href passed to <Link>, router.prefetch(), or similar
 * @returns A URL object to prefetch, or null if prefetching should be disabled
 */ export function createPrefetchURL(href) {
    // Don't prefetch for bots as they don't navigate.
    if (isBot(window.navigator.userAgent)) {
        return null;
    }
    let url;
    try {
        url = new URL(addBasePath(href), window.location.href);
    } catch (_) {
        // TODO: Does this need to throw or can we just console.error instead? Does
        // anyone rely on this throwing? (Seems unlikely.)
        throw new Error("Cannot prefetch '" + href + "' because it cannot be converted to a URL.");
    }
    // Don't prefetch during development (improves compilation performance)
    if (process.env.NODE_ENV === 'development') {
        return null;
    }
    // External urls can't be prefetched in the same way.
    if (isExternalURL(url)) {
        return null;
    }
    return url;
}
function HistoryUpdater(param) {
    let { appRouterState } = param;
    useInsertionEffect(()=>{
        if (process.env.__NEXT_APP_NAV_FAIL_HANDLING) {
            // clear pending URL as navigation is no longer
            // in flight
            window.next.__pendingUrl = undefined;
        }
        const { tree, pushRef, canonicalUrl } = appRouterState;
        const historyState = {
            ...pushRef.preserveCustomHistoryState ? window.history.state : {},
            // Identifier is shortened intentionally.
            // __NA is used to identify if the history entry can be handled by the app-router.
            // __N is used to identify if the history entry can be handled by the old router.
            __NA: true,
            __PRIVATE_NEXTJS_INTERNALS_TREE: tree
        };
        if (pushRef.pendingPush && // Skip pushing an additional history entry if the canonicalUrl is the same as the current url.
        // This mirrors the browser behavior for normal navigation.
        createHrefFromUrl(new URL(window.location.href)) !== canonicalUrl) {
            // This intentionally mutates React state, pushRef is overwritten to ensure additional push/replace calls do not trigger an additional history entry.
            pushRef.pendingPush = false;
            window.history.pushState(historyState, '', canonicalUrl);
        } else {
            window.history.replaceState(historyState, '', canonicalUrl);
        }
    }, [
        appRouterState
    ]);
    return null;
}
export function createEmptyCacheNode() {
    return {
        lazyData: null,
        rsc: null,
        prefetchRsc: null,
        head: null,
        prefetchHead: null,
        parallelRoutes: new Map(),
        loading: null
    };
}
/**
 * Server response that only patches the cache and tree.
 */ function useChangeByServerResponse(dispatch) {
    return useCallback((param)=>{
        let { previousTree, serverResponse } = param;
        startTransition(()=>{
            dispatch({
                type: ACTION_SERVER_PATCH,
                previousTree,
                serverResponse
            });
        });
    }, [
        dispatch
    ]);
}
function useNavigate(dispatch) {
    return useCallback((href, navigateType, shouldScroll)=>{
        const url = new URL(addBasePath(href), location.href);
        if (process.env.__NEXT_APP_NAV_FAIL_HANDLING) {
            window.next.__pendingUrl = url;
        }
        return dispatch({
            type: ACTION_NAVIGATE,
            url,
            isExternalUrl: isExternalURL(url),
            locationSearch: location.search,
            shouldScroll: shouldScroll != null ? shouldScroll : true,
            navigateType,
            allowAliasing: true
        });
    }, [
        dispatch
    ]);
}
function copyNextJsInternalHistoryState(data) {
    if (data == null) data = {};
    const currentState = window.history.state;
    const __NA = currentState == null ? void 0 : currentState.__NA;
    if (__NA) {
        data.__NA = __NA;
    }
    const __PRIVATE_NEXTJS_INTERNALS_TREE = currentState == null ? void 0 : currentState.__PRIVATE_NEXTJS_INTERNALS_TREE;
    if (__PRIVATE_NEXTJS_INTERNALS_TREE) {
        data.__PRIVATE_NEXTJS_INTERNALS_TREE = __PRIVATE_NEXTJS_INTERNALS_TREE;
    }
    return data;
}
function Head(param) {
    let { headCacheNode } = param;
    // If this segment has a `prefetchHead`, it's the statically prefetched data.
    // We should use that on initial render instead of `head`. Then we'll switch
    // to `head` when the dynamic response streams in.
    const head = headCacheNode !== null ? headCacheNode.head : null;
    const prefetchHead = headCacheNode !== null ? headCacheNode.prefetchHead : null;
    // If no prefetch data is available, then we go straight to rendering `head`.
    const resolvedPrefetchRsc = prefetchHead !== null ? prefetchHead : head;
    // We use `useDeferredValue` to handle switching between the prefetched and
    // final values. The second argument is returned on initial render, then it
    // re-renders with the first argument.
    //
    // @ts-expect-error The second argument to `useDeferredValue` is only
    // available in the experimental builds. When its disabled, it will always
    // return `head`.
    return useDeferredValue(head, resolvedPrefetchRsc);
}
/**
 * The global router that wraps the application components.
 */ function Router(param) {
    let { actionQueue, assetPrefix } = param;
    const [state, dispatch] = useReducer(actionQueue);
    const { canonicalUrl } = useUnwrapState(state);
    // Add memoized pathname/query for useSearchParams and usePathname.
    const { searchParams, pathname } = useMemo(()=>{
        const url = new URL(canonicalUrl, typeof window === 'undefined' ? 'http://n' : window.location.href);
        return {
            // This is turned into a readonly class in `useSearchParams`
            searchParams: url.searchParams,
            pathname: hasBasePath(url.pathname) ? removeBasePath(url.pathname) : url.pathname
        };
    }, [
        canonicalUrl
    ]);
    const changeByServerResponse = useChangeByServerResponse(dispatch);
    const navigate = useNavigate(dispatch);
    useServerActionDispatcher(dispatch);
    /**
   * The app router that is exposed through `useRouter`. It's only concerned with dispatching actions to the reducer, does not hold state.
   */ const appRouter = useMemo(()=>{
        const routerInstance = {
            back: ()=>window.history.back(),
            forward: ()=>window.history.forward(),
            prefetch: process.env.__NEXT_PPR && process.env.__NEXT_CLIENT_SEGMENT_CACHE ? // data in the router reducer state; it writes into a global mutable
            // cache. So we don't need to dispatch an action.
            (href)=>prefetchWithSegmentCache(href, actionQueue.state.nextUrl) : (href, options)=>{
                // Use the old prefetch implementation.
                const url = createPrefetchURL(href);
                if (url !== null) {
                    startTransition(()=>{
                        var _options_kind;
                        dispatch({
                            type: ACTION_PREFETCH,
                            url,
                            kind: (_options_kind = options == null ? void 0 : options.kind) != null ? _options_kind : PrefetchKind.FULL
                        });
                    });
                }
            },
            replace: (href, options)=>{
                if (options === void 0) options = {};
                startTransition(()=>{
                    var _options_scroll;
                    navigate(href, 'replace', (_options_scroll = options.scroll) != null ? _options_scroll : true);
                });
            },
            push: (href, options)=>{
                if (options === void 0) options = {};
                startTransition(()=>{
                    var _options_scroll;
                    navigate(href, 'push', (_options_scroll = options.scroll) != null ? _options_scroll : true);
                });
            },
            refresh: ()=>{
                startTransition(()=>{
                    dispatch({
                        type: ACTION_REFRESH,
                        origin: window.location.origin
                    });
                });
            },
            hmrRefresh: ()=>{
                if (process.env.NODE_ENV !== 'development') {
                    throw new Error('hmrRefresh can only be used in development mode. Please use refresh instead.');
                } else {
                    startTransition(()=>{
                        dispatch({
                            type: ACTION_HMR_REFRESH,
                            origin: window.location.origin
                        });
                    });
                }
            }
        };
        return routerInstance;
    }, [
        actionQueue,
        dispatch,
        navigate
    ]);
    useEffect(()=>{
        // Exists for debugging purposes. Don't use in application code.
        if (window.next) {
            window.next.router = appRouter;
        }
    }, [
        appRouter
    ]);
    if (process.env.NODE_ENV !== 'production') {
        // eslint-disable-next-line react-hooks/rules-of-hooks
        const { cache, prefetchCache, tree } = useUnwrapState(state);
        // This hook is in a conditional but that is ok because `process.env.NODE_ENV` never changes
        // eslint-disable-next-line react-hooks/rules-of-hooks
        useEffect(()=>{
            // Add `window.nd` for debugging purposes.
            // This is not meant for use in applications as concurrent rendering will affect the cache/tree/router.
            // @ts-ignore this is for debugging
            window.nd = {
                router: appRouter,
                cache,
                prefetchCache,
                tree
            };
        }, [
            appRouter,
            cache,
            prefetchCache,
            tree
        ]);
    }
    useEffect(()=>{
        // If the app is restored from bfcache, it's possible that
        // pushRef.mpaNavigation is true, which would mean that any re-render of this component
        // would trigger the mpa navigation logic again from the lines below.
        // This will restore the router to the initial state in the event that the app is restored from bfcache.
        function handlePageShow(event) {
            var _window_history_state;
            if (!event.persisted || !((_window_history_state = window.history.state) == null ? void 0 : _window_history_state.__PRIVATE_NEXTJS_INTERNALS_TREE)) {
                return;
            }
            // Clear the pendingMpaPath value so that a subsequent MPA navigation to the same URL can be triggered.
            // This is necessary because if the browser restored from bfcache, the pendingMpaPath would still be set to the value
            // of the last MPA navigation.
            globalMutable.pendingMpaPath = undefined;
            dispatch({
                type: ACTION_RESTORE,
                url: new URL(window.location.href),
                tree: window.history.state.__PRIVATE_NEXTJS_INTERNALS_TREE
            });
        }
        window.addEventListener('pageshow', handlePageShow);
        return ()=>{
            window.removeEventListener('pageshow', handlePageShow);
        };
    }, [
        dispatch
    ]);
    useEffect(()=>{
        // Ensure that any redirect errors that bubble up outside of the RedirectBoundary
        // are caught and handled by the router.
        function handleUnhandledRedirect(event) {
            const error = 'reason' in event ? event.reason : event.error;
            if (isRedirectError(error)) {
                event.preventDefault();
                const url = getURLFromRedirectError(error);
                const redirectType = getRedirectTypeFromError(error);
                if (redirectType === RedirectType.push) {
                    appRouter.push(url, {});
                } else {
                    appRouter.replace(url, {});
                }
            }
        }
        window.addEventListener('error', handleUnhandledRedirect);
        window.addEventListener('unhandledrejection', handleUnhandledRedirect);
        return ()=>{
            window.removeEventListener('error', handleUnhandledRedirect);
            window.removeEventListener('unhandledrejection', handleUnhandledRedirect);
        };
    }, [
        appRouter
    ]);
    // When mpaNavigation flag is set do a hard navigation to the new url.
    // Infinitely suspend because we don't actually want to rerender any child
    // components with the new URL and any entangled state updates shouldn't
    // commit either (eg: useTransition isPending should stay true until the page
    // unloads).
    //
    // This is a side effect in render. Don't try this at home, kids. It's
    // probably safe because we know this is a singleton component and it's never
    // in <Offscreen>. At least I hope so. (It will run twice in dev strict mode,
    // but that's... fine?)
    const { pushRef } = useUnwrapState(state);
    if (pushRef.mpaNavigation) {
        // if there's a re-render, we don't want to trigger another redirect if one is already in flight to the same URL
        if (globalMutable.pendingMpaPath !== canonicalUrl) {
            const location1 = window.location;
            if (pushRef.pendingPush) {
                location1.assign(canonicalUrl);
            } else {
                location1.replace(canonicalUrl);
            }
            globalMutable.pendingMpaPath = canonicalUrl;
        }
        // TODO-APP: Should we listen to navigateerror here to catch failed
        // navigations somehow? And should we call window.stop() if a SPA navigation
        // should interrupt an MPA one?
        use(unresolvedThenable);
    }
    useEffect(()=>{
        const originalPushState = window.history.pushState.bind(window.history);
        const originalReplaceState = window.history.replaceState.bind(window.history);
        // Ensure the canonical URL in the Next.js Router is updated when the URL is changed so that `usePathname` and `useSearchParams` hold the pushed values.
        const applyUrlFromHistoryPushReplace = (url)=>{
            var _window_history_state;
            const href = window.location.href;
            const tree = (_window_history_state = window.history.state) == null ? void 0 : _window_history_state.__PRIVATE_NEXTJS_INTERNALS_TREE;
            startTransition(()=>{
                dispatch({
                    type: ACTION_RESTORE,
                    url: new URL(url != null ? url : href, href),
                    tree
                });
            });
        };
        /**
     * Patch pushState to ensure external changes to the history are reflected in the Next.js Router.
     * Ensures Next.js internal history state is copied to the new history entry.
     * Ensures usePathname and useSearchParams hold the newly provided url.
     */ window.history.pushState = function pushState(data, _unused, url) {
            // Avoid a loop when Next.js internals trigger pushState/replaceState
            if ((data == null ? void 0 : data.__NA) || (data == null ? void 0 : data._N)) {
                return originalPushState(data, _unused, url);
            }
            data = copyNextJsInternalHistoryState(data);
            if (url) {
                applyUrlFromHistoryPushReplace(url);
            }
            return originalPushState(data, _unused, url);
        };
        /**
     * Patch replaceState to ensure external changes to the history are reflected in the Next.js Router.
     * Ensures Next.js internal history state is copied to the new history entry.
     * Ensures usePathname and useSearchParams hold the newly provided url.
     */ window.history.replaceState = function replaceState(data, _unused, url) {
            // Avoid a loop when Next.js internals trigger pushState/replaceState
            if ((data == null ? void 0 : data.__NA) || (data == null ? void 0 : data._N)) {
                return originalReplaceState(data, _unused, url);
            }
            data = copyNextJsInternalHistoryState(data);
            if (url) {
                applyUrlFromHistoryPushReplace(url);
            }
            return originalReplaceState(data, _unused, url);
        };
        /**
     * Handle popstate event, this is used to handle back/forward in the browser.
     * By default dispatches ACTION_RESTORE, however if the history entry was not pushed/replaced by app-router it will reload the page.
     * That case can happen when the old router injected the history entry.
     */ const onPopState = (event)=>{
            if (!event.state) {
                // TODO-APP: this case only happens when pushState/replaceState was called outside of Next.js. It should probably reload the page in this case.
                return;
            }
            // This case happens when the history entry was pushed by the `pages` router.
            if (!event.state.__NA) {
                window.location.reload();
                return;
            }
            // TODO-APP: Ideally the back button should not use startTransition as it should apply the updates synchronously
            // Without startTransition works if the cache is there for this path
            startTransition(()=>{
                dispatch({
                    type: ACTION_RESTORE,
                    url: new URL(window.location.href),
                    tree: event.state.__PRIVATE_NEXTJS_INTERNALS_TREE
                });
            });
        };
        // Register popstate event to call onPopstate.
        window.addEventListener('popstate', onPopState);
        return ()=>{
            window.history.pushState = originalPushState;
            window.history.replaceState = originalReplaceState;
            window.removeEventListener('popstate', onPopState);
        };
    }, [
        dispatch
    ]);
    const { cache, tree, nextUrl, focusAndScrollRef } = useUnwrapState(state);
    const matchingHead = useMemo(()=>{
        return findHeadInCache(cache, tree[1]);
    }, [
        cache,
        tree
    ]);
    // Add memoized pathParams for useParams.
    const pathParams = useMemo(()=>{
        return getSelectedParams(tree);
    }, [
        tree
    ]);
    const layoutRouterContext = useMemo(()=>{
        return {
            childNodes: cache.parallelRoutes,
            tree,
            // Root node always has `url`
            // Provided in AppTreeContext to ensure it can be overwritten in layout-router
            url: canonicalUrl,
            loading: cache.loading
        };
    }, [
        cache.parallelRoutes,
        tree,
        canonicalUrl,
        cache.loading
    ]);
    const globalLayoutRouterContext = useMemo(()=>{
        return {
            changeByServerResponse,
            tree,
            focusAndScrollRef,
            nextUrl
        };
    }, [
        changeByServerResponse,
        tree,
        focusAndScrollRef,
        nextUrl
    ]);
    let head;
    if (matchingHead !== null) {
        // The head is wrapped in an extra component so we can use
        // `useDeferredValue` to swap between the prefetched and final versions of
        // the head. (This is what LayoutRouter does for segment data, too.)
        //
        // The `key` is used to remount the component whenever the head moves to
        // a different segment.
        const [headCacheNode, headKey] = matchingHead;
        head = /*#__PURE__*/ _jsx(Head, {
            headCacheNode: headCacheNode
        }, headKey);
    } else {
        head = null;
    }
    let content = /*#__PURE__*/ _jsxs(RedirectBoundary, {
        children: [
            head,
            cache.rsc,
            /*#__PURE__*/ _jsx(AppRouterAnnouncer, {
                tree: tree
            })
        ]
    });
    if (process.env.NODE_ENV !== 'production') {
        if (typeof window !== 'undefined') {
            const { DevRootHTTPAccessFallbackBoundary } = require('./dev-root-http-access-fallback-boundary');
            content = /*#__PURE__*/ _jsx(DevRootHTTPAccessFallbackBoundary, {
                children: content
            });
        }
        const HotReloader = require('./react-dev-overlay/app/hot-reloader-client').default;
        content = /*#__PURE__*/ _jsx(HotReloader, {
            assetPrefix: assetPrefix,
            children: content
        });
    }
    return /*#__PURE__*/ _jsxs(_Fragment, {
        children: [
            /*#__PURE__*/ _jsx(HistoryUpdater, {
                appRouterState: useUnwrapState(state)
            }),
            /*#__PURE__*/ _jsx(RuntimeStyles, {}),
            /*#__PURE__*/ _jsx(PathParamsContext.Provider, {
                value: pathParams,
                children: /*#__PURE__*/ _jsx(PathnameContext.Provider, {
                    value: pathname,
                    children: /*#__PURE__*/ _jsx(SearchParamsContext.Provider, {
                        value: searchParams,
                        children: /*#__PURE__*/ _jsx(GlobalLayoutRouterContext.Provider, {
                            value: globalLayoutRouterContext,
                            children: /*#__PURE__*/ _jsx(AppRouterContext.Provider, {
                                value: appRouter,
                                children: /*#__PURE__*/ _jsx(LayoutRouterContext.Provider, {
                                    value: layoutRouterContext,
                                    children: content
                                })
                            })
                        })
                    })
                })
            })
        ]
    });
}
export default function AppRouter(param) {
    let { actionQueue, globalErrorComponentAndStyles: [globalErrorComponent, globalErrorStyles], assetPrefix } = param;
    useNavFailureHandler();
    return /*#__PURE__*/ _jsx(ErrorBoundary, {
        errorComponent: globalErrorComponent,
        errorStyles: globalErrorStyles,
        children: /*#__PURE__*/ _jsx(Router, {
            actionQueue: actionQueue,
            assetPrefix: assetPrefix
        })
    });
}
const runtimeStyles = new Set();
let runtimeStyleChanged = new Set();
globalThis._N_E_STYLE_LOAD = function(href) {
    let len = runtimeStyles.size;
    runtimeStyles.add(href);
    if (runtimeStyles.size !== len) {
        runtimeStyleChanged.forEach((cb)=>cb());
    }
    // TODO figure out how to get a promise here
    // But maybe it's not necessary as react would block rendering until it's loaded
    return Promise.resolve();
};
function RuntimeStyles() {
    const [, forceUpdate] = React.useState(0);
    const renderedStylesSize = runtimeStyles.size;
    useEffect(()=>{
        const changed = ()=>forceUpdate((c)=>c + 1);
        runtimeStyleChanged.add(changed);
        if (renderedStylesSize !== runtimeStyles.size) {
            changed();
        }
        return ()=>{
            runtimeStyleChanged.delete(changed);
        };
    }, [
        renderedStylesSize,
        forceUpdate
    ]);
    const dplId = process.env.NEXT_DEPLOYMENT_ID ? "?dpl=" + process.env.NEXT_DEPLOYMENT_ID : '';
    return [
        ...runtimeStyles
    ].map((href, i)=>/*#__PURE__*/ _jsx("link", {
            rel: "stylesheet",
            href: "" + href + dplId,
            // @ts-ignore
            precedence: "next"
        }, i));
}

//# sourceMappingURL=app-router.js.map