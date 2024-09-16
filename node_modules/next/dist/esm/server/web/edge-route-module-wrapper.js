import "./globals";
import { adapter } from "./adapter";
import { IncrementalCache } from "../lib/incremental-cache";
import { RouteMatcher } from "../future/route-matchers/route-matcher";
import { internal_getCurrentFunctionWaitUntil } from "./internal-edge-wait-until";
import { getUtils } from "../server-utils";
import { searchParamsToUrlQuery } from "../../shared/lib/router/utils/querystring";
import { getEdgePreviewProps } from "./get-edge-preview-props";
/**
 * EdgeRouteModuleWrapper is a wrapper around a route module.
 *
 * Note that this class should only be used in the edge runtime.
 */ export class EdgeRouteModuleWrapper {
    /**
   * The constructor is wrapped with private to ensure that it can only be
   * constructed by the static wrap method.
   *
   * @param routeModule the route module to wrap
   */ constructor(routeModule){
        this.routeModule = routeModule;
        // TODO: (wyattjoh) possibly allow the module to define it's own matcher
        this.matcher = new RouteMatcher(routeModule.definition);
    }
    /**
   * This will wrap a module with the EdgeModuleWrapper and return a function
   * that can be used as a handler for the edge runtime.
   *
   * @param module the module to wrap
   * @param options any options that should be passed to the adapter and
   *                override the ones passed from the runtime
   * @returns a function that can be used as a handler for the edge runtime
   */ static wrap(routeModule, options = {}) {
        // Create the module wrapper.
        const wrapper = new EdgeRouteModuleWrapper(routeModule);
        // Return the wrapping function.
        return (opts)=>{
            return adapter({
                ...opts,
                ...options,
                IncrementalCache,
                // Bind the handler method to the wrapper so it still has context.
                handler: wrapper.handler.bind(wrapper)
            });
        };
    }
    async handler(request, evt) {
        const utils = getUtils({
            pageIsDynamic: this.matcher.isDynamic,
            page: this.matcher.definition.pathname,
            basePath: request.nextUrl.basePath,
            // We don't need the `handleRewrite` util, so can just pass an empty object
            rewrites: {},
            // only used for rewrites, so setting an arbitrary default value here
            caseSensitive: false
        });
        const { params } = utils.normalizeDynamicRouteParams(searchParamsToUrlQuery(request.nextUrl.searchParams));
        const previewProps = getEdgePreviewProps();
        // Create the context for the handler. This contains the params from the
        // match (if any).
        const context = {
            params,
            prerenderManifest: {
                version: 4,
                routes: {},
                dynamicRoutes: {},
                preview: previewProps,
                notFoundRoutes: []
            },
            renderOpts: {
                supportsDynamicHTML: true,
                // App Route's cannot be postponed.
                experimental: {
                    ppr: false
                }
            }
        };
        // Get the response from the handler.
        const res = await this.routeModule.handle(request, context);
        const waitUntilPromises = [
            internal_getCurrentFunctionWaitUntil()
        ];
        if (context.renderOpts.waitUntil) {
            waitUntilPromises.push(context.renderOpts.waitUntil);
        }
        evt.waitUntil(Promise.all(waitUntilPromises));
        return res;
    }
}

//# sourceMappingURL=edge-route-module-wrapper.js.map