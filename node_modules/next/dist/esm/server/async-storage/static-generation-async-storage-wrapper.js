import { createPrerenderState } from "../../server/app-render/dynamic-rendering";
export const StaticGenerationAsyncStorageWrapper = {
    wrap (storage, { urlPathname, renderOpts, requestEndedState }, callback) {
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
     *    3.) If the request is in draft mode, we must generate dynamic HTML.
     *
     *    4.) If the request is a server action, we must generate dynamic HTML.
     *
     * These rules help ensure that other existing features like request caching,
     * coalescing, and ISR continue working as intended.
     */ const isStaticGeneration = !renderOpts.supportsDynamicHTML && !renderOpts.isDraftMode && !renderOpts.isServerAction;
        const prerenderState = isStaticGeneration && renderOpts.experimental.ppr ? createPrerenderState(renderOpts.isDebugPPRSkeleton) : null;
        const store = {
            isStaticGeneration,
            urlPathname,
            pagePath: renderOpts.originalPathname,
            incrementalCache: // we fallback to a global incremental cache for edge-runtime locally
            // so that it can access the fs cache without mocks
            renderOpts.incrementalCache || globalThis.__incrementalCache,
            isRevalidate: renderOpts.isRevalidate,
            isPrerendering: renderOpts.nextExport,
            fetchCache: renderOpts.fetchCache,
            isOnDemandRevalidate: renderOpts.isOnDemandRevalidate,
            isDraftMode: renderOpts.isDraftMode,
            prerenderState,
            requestEndedState
        };
        // TODO: remove this when we resolve accessing the store outside the execution context
        renderOpts.store = store;
        return storage.run(store, callback, store);
    }
};

//# sourceMappingURL=static-generation-async-storage-wrapper.js.map