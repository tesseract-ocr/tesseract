import { createHrefFromUrl } from "../create-href-from-url";
import { extractPathFromFlightRouterState } from "../compute-changed-path";
import { updateCacheNodeOnPopstateRestoration } from "../ppr-navigations";
export function restoreReducer(state, action) {
    const { url, tree } = action;
    const href = createHrefFromUrl(url);
    // This action is used to restore the router state from the history state.
    // However, it's possible that the history state no longer contains the `FlightRouterState`.
    // We will copy over the internal state on pushState/replaceState events, but if a history entry
    // occurred before hydration, or if the user navigated to a hash using a regular anchor link,
    // the history state will not contain the `FlightRouterState`.
    // In this case, we'll continue to use the existing tree so the router doesn't get into an invalid state.
    const treeToRestore = tree || state.tree;
    const oldCache = state.cache;
    const newCache = process.env.__NEXT_PPR ? // data for any segment whose dynamic data was already received. This
    // prevents an unnecessary flash back to PPR state during a
    // back/forward navigation.
    updateCacheNodeOnPopstateRestoration(oldCache, treeToRestore) : oldCache;
    var _extractPathFromFlightRouterState;
    return {
        buildId: state.buildId,
        // Set canonical url
        canonicalUrl: href,
        pushRef: {
            pendingPush: false,
            mpaNavigation: false,
            // Ensures that the custom history state that was set is preserved when applying this update.
            preserveCustomHistoryState: true
        },
        focusAndScrollRef: state.focusAndScrollRef,
        cache: newCache,
        prefetchCache: state.prefetchCache,
        // Restore provided tree
        tree: treeToRestore,
        nextUrl: (_extractPathFromFlightRouterState = extractPathFromFlightRouterState(treeToRestore)) != null ? _extractPathFromFlightRouterState : url.pathname
    };
}

//# sourceMappingURL=restore-reducer.js.map