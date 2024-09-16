import { RouteModule } from "../route-module";
import { RequestAsyncStorageWrapper } from "../../../async-storage/request-async-storage-wrapper";
import { StaticGenerationAsyncStorageWrapper } from "../../../async-storage/static-generation-async-storage-wrapper";
import { handleBadRequestResponse, handleInternalServerErrorResponse } from "../helpers/response-handlers";
import { HTTP_METHODS, isHTTPMethod } from "../../../web/http";
import { addImplicitTags, patchFetch } from "../../../lib/patch-fetch";
import { getTracer } from "../../../lib/trace/tracer";
import { AppRouteRouteHandlersSpan } from "../../../lib/trace/constants";
import { getPathnameFromAbsolutePath } from "./helpers/get-pathname-from-absolute-path";
import { resolveHandlerError } from "./helpers/resolve-handler-error";
import * as Log from "../../../../build/output/log";
import { autoImplementMethods } from "./helpers/auto-implement-methods";
import { appendMutableCookies } from "../../../web/spec-extension/adapters/request-cookies";
import { HeadersAdapter } from "../../../web/spec-extension/adapters/headers";
import { RequestCookiesAdapter } from "../../../web/spec-extension/adapters/request-cookies";
import { parsedUrlQueryToParams } from "./helpers/parsed-url-query-to-params";
import * as serverHooks from "../../../../client/components/hooks-server-context";
import { DynamicServerError } from "../../../../client/components/hooks-server-context";
import { requestAsyncStorage } from "../../../../client/components/request-async-storage.external";
import { staticGenerationAsyncStorage } from "../../../../client/components/static-generation-async-storage.external";
import { actionAsyncStorage } from "../../../../client/components/action-async-storage.external";
import * as sharedModules from "./shared-modules";
import { getIsServerAction } from "../../../lib/server-action-request-meta";
import { RequestCookies } from "next/dist/compiled/@edge-runtime/cookies";
import { cleanURL } from "./helpers/clean-url";
import { StaticGenBailoutError } from "../../../../client/components/static-generation-bailout";
import { trackDynamicDataAccessed } from "../../../app-render/dynamic-rendering";
import { ReflectAdapter } from "../../../web/spec-extension/adapters/reflect";
/**
 * AppRouteRouteHandler is the handler for app routes.
 */ export class AppRouteRouteModule extends RouteModule {
    static #_ = this.sharedModules = sharedModules;
    constructor({ userland, definition, resolvedPagePath, nextConfigOutput }){
        super({
            userland,
            definition
        });
        /**
   * A reference to the request async storage.
   */ this.requestAsyncStorage = requestAsyncStorage;
        /**
   * A reference to the static generation async storage.
   */ this.staticGenerationAsyncStorage = staticGenerationAsyncStorage;
        /**
   * An interface to call server hooks which interact with the underlying
   * storage.
   */ this.serverHooks = serverHooks;
        /**
   * A reference to the mutation related async storage, such as mutations of
   * cookies.
   */ this.actionAsyncStorage = actionAsyncStorage;
        this.resolvedPagePath = resolvedPagePath;
        this.nextConfigOutput = nextConfigOutput;
        // Automatically implement some methods if they aren't implemented by the
        // userland module.
        this.methods = autoImplementMethods(userland);
        // Get the non-static methods for this route.
        this.hasNonStaticMethods = hasNonStaticMethods(userland);
        // Get the dynamic property from the userland module.
        this.dynamic = this.userland.dynamic;
        if (this.nextConfigOutput === "export") {
            if (!this.dynamic || this.dynamic === "auto") {
                this.dynamic = "error";
            } else if (this.dynamic === "force-dynamic") {
                throw new Error(`export const dynamic = "force-dynamic" on page "${definition.pathname}" cannot be used with "output: export". See more info here: https://nextjs.org/docs/advanced-features/static-html-export`);
            }
        }
        // We only warn in development after here, so return if we're not in
        // development.
        if (process.env.NODE_ENV === "development") {
            // Print error in development if the exported handlers are in lowercase, only
            // uppercase handlers are supported.
            const lowercased = HTTP_METHODS.map((method)=>method.toLowerCase());
            for (const method of lowercased){
                if (method in this.userland) {
                    Log.error(`Detected lowercase method '${method}' in '${this.resolvedPagePath}'. Export the uppercase '${method.toUpperCase()}' method name to fix this error.`);
                }
            }
            // Print error if the module exports a default handler, they must use named
            // exports for each HTTP method.
            if ("default" in this.userland) {
                Log.error(`Detected default export in '${this.resolvedPagePath}'. Export a named export for each HTTP method instead.`);
            }
            // If there is no methods exported by this module, then return a not found
            // response.
            if (!HTTP_METHODS.some((method)=>method in this.userland)) {
                Log.error(`No HTTP methods exported in '${this.resolvedPagePath}'. Export a named export for each HTTP method.`);
            }
        }
    }
    /**
   * Resolves the handler function for the given method.
   *
   * @param method the requested method
   * @returns the handler function for the given method
   */ resolve(method) {
        // Ensure that the requested method is a valid method (to prevent RCE's).
        if (!isHTTPMethod(method)) return handleBadRequestResponse;
        // Return the handler.
        return this.methods[method];
    }
    /**
   * Executes the route handler.
   */ async execute(rawRequest, context) {
        // Get the handler function for the given method.
        const handler = this.resolve(rawRequest.method);
        // Get the context for the request.
        const requestContext = {
            req: rawRequest
        };
        requestContext.renderOpts = {
            previewProps: context.prerenderManifest.preview
        };
        // Get the context for the static generation.
        const staticGenerationContext = {
            urlPathname: rawRequest.nextUrl.pathname,
            renderOpts: context.renderOpts
        };
        // Add the fetchCache option to the renderOpts.
        staticGenerationContext.renderOpts.fetchCache = this.userland.fetchCache;
        // Run the handler with the request AsyncLocalStorage to inject the helper
        // support. We set this to `unknown` because the type is not known until
        // runtime when we do a instanceof check below.
        const response = await this.actionAsyncStorage.run({
            isAppRoute: true,
            isAction: getIsServerAction(rawRequest)
        }, ()=>RequestAsyncStorageWrapper.wrap(this.requestAsyncStorage, requestContext, ()=>StaticGenerationAsyncStorageWrapper.wrap(this.staticGenerationAsyncStorage, staticGenerationContext, (staticGenerationStore)=>{
                    var _getTracer_getRootSpanAttributes;
                    // Check to see if we should bail out of static generation based on
                    // having non-static methods.
                    const isStaticGeneration = staticGenerationStore.isStaticGeneration;
                    if (this.hasNonStaticMethods) {
                        if (isStaticGeneration) {
                            const err = new DynamicServerError("Route is configured with methods that cannot be statically generated.");
                            staticGenerationStore.dynamicUsageDescription = err.message;
                            staticGenerationStore.dynamicUsageStack = err.stack;
                            throw err;
                        } else {
                            // We aren't statically generating but since this route has non-static methods
                            // we still need to set the default caching to no cache by setting revalidate = 0
                            // @TODO this type of logic is too indirect. we need to refactor how we set fetch cache
                            // behavior. Prior to the most recent refactor this logic was buried deep in staticGenerationBailout
                            // so it is possible it was unintentional and then tests were written to assert the current behavior
                            staticGenerationStore.revalidate = 0;
                        }
                    }
                    // We assume we can pass the original request through however we may end up
                    // proxying it in certain circumstances based on execution type and configuration
                    let request = rawRequest;
                    // Update the static generation store based on the dynamic property.
                    switch(this.dynamic){
                        case "force-dynamic":
                            {
                                // Routes of generated paths should be dynamic
                                staticGenerationStore.forceDynamic = true;
                                break;
                            }
                        case "force-static":
                            // The dynamic property is set to force-static, so we should
                            // force the page to be static.
                            staticGenerationStore.forceStatic = true;
                            // We also Proxy the request to replace dynamic data on the request
                            // with empty stubs to allow for safely executing as static
                            request = new Proxy(rawRequest, forceStaticRequestHandlers);
                            break;
                        case "error":
                            // The dynamic property is set to error, so we should throw an
                            // error if the page is being statically generated.
                            staticGenerationStore.dynamicShouldError = true;
                            if (isStaticGeneration) request = new Proxy(rawRequest, requireStaticRequestHandlers);
                            break;
                        default:
                            // We proxy `NextRequest` to track dynamic access, and potentially bail out of static generation
                            request = proxyNextRequest(rawRequest, staticGenerationStore);
                    }
                    // If the static generation store does not have a revalidate value
                    // set, then we should set it the revalidate value from the userland
                    // module or default to false.
                    staticGenerationStore.revalidate ??= this.userland.revalidate ?? false;
                    // TODO: propagate this pathname from route matcher
                    const route = getPathnameFromAbsolutePath(this.resolvedPagePath);
                    (_getTracer_getRootSpanAttributes = getTracer().getRootSpanAttributes()) == null ? void 0 : _getTracer_getRootSpanAttributes.set("next.route", route);
                    return getTracer().trace(AppRouteRouteHandlersSpan.runHandler, {
                        spanName: `executing api route (app) ${route}`,
                        attributes: {
                            "next.route": route
                        }
                    }, async ()=>{
                        var _staticGenerationStore_incrementalCache, _staticGenerationStore_tags;
                        // Patch the global fetch.
                        patchFetch({
                            serverHooks: this.serverHooks,
                            staticGenerationAsyncStorage: this.staticGenerationAsyncStorage
                        });
                        const res = await handler(request, {
                            params: context.params ? parsedUrlQueryToParams(context.params) : undefined
                        });
                        if (!(res instanceof Response)) {
                            throw new Error(`No response is returned from route handler '${this.resolvedPagePath}'. Ensure you return a \`Response\` or a \`NextResponse\` in all branches of your handler.`);
                        }
                        context.renderOpts.fetchMetrics = staticGenerationStore.fetchMetrics;
                        context.renderOpts.waitUntil = Promise.all([
                            (_staticGenerationStore_incrementalCache = staticGenerationStore.incrementalCache) == null ? void 0 : _staticGenerationStore_incrementalCache.revalidateTag(staticGenerationStore.revalidatedTags || []),
                            ...Object.values(staticGenerationStore.pendingRevalidates || {})
                        ]);
                        addImplicitTags(staticGenerationStore);
                        context.renderOpts.fetchTags = (_staticGenerationStore_tags = staticGenerationStore.tags) == null ? void 0 : _staticGenerationStore_tags.join(",");
                        // It's possible cookies were set in the handler, so we need
                        // to merge the modified cookies and the returned response
                        // here.
                        const requestStore = this.requestAsyncStorage.getStore();
                        if (requestStore && requestStore.mutableCookies) {
                            const headers = new Headers(res.headers);
                            if (appendMutableCookies(headers, requestStore.mutableCookies)) {
                                return new Response(res.body, {
                                    status: res.status,
                                    statusText: res.statusText,
                                    headers
                                });
                            }
                        }
                        return res;
                    });
                })));
        // If the handler did't return a valid response, then return the internal
        // error response.
        if (!(response instanceof Response)) {
            // TODO: validate the correct handling behavior, maybe log something?
            return handleInternalServerErrorResponse();
        }
        if (response.headers.has("x-middleware-rewrite")) {
            // TODO: move this error into the `NextResponse.rewrite()` function.
            // TODO-APP: re-enable support below when we can proxy these type of requests
            throw new Error("NextResponse.rewrite() was used in a app route handler, this is not currently supported. Please remove the invocation to continue.");
        // // This is a rewrite created via `NextResponse.rewrite()`. We need to send
        // // the response up so it can be handled by the backing server.
        // // If the server is running in minimal mode, we just want to forward the
        // // response (including the rewrite headers) upstream so it can perform the
        // // redirect for us, otherwise return with the special condition so this
        // // server can perform a rewrite.
        // if (!minimalMode) {
        //   return { response, condition: 'rewrite' }
        // }
        // // Relativize the url so it's relative to the base url. This is so the
        // // outgoing headers upstream can be relative.
        // const rewritePath = response.headers.get('x-middleware-rewrite')!
        // const initUrl = getRequestMeta(req, 'initURL')!
        // const { pathname } = parseUrl(relativizeURL(rewritePath, initUrl))
        // response.headers.set('x-middleware-rewrite', pathname)
        }
        if (response.headers.get("x-middleware-next") === "1") {
            // TODO: move this error into the `NextResponse.next()` function.
            throw new Error("NextResponse.next() was used in a app route handler, this is not supported. See here for more info: https://nextjs.org/docs/messages/next-response-next-in-app-route-handler");
        }
        return response;
    }
    async handle(request, context) {
        try {
            // Execute the route to get the response.
            const response = await this.execute(request, context);
            // The response was handled, return it.
            return response;
        } catch (err) {
            // Try to resolve the error to a response, else throw it again.
            const response = resolveHandlerError(err);
            if (!response) throw err;
            // The response was resolved, return it.
            return response;
        }
    }
}
export default AppRouteRouteModule;
/**
 * Gets all the method names for handlers that are not considered static.
 *
 * @param handlers the handlers from the userland module
 * @returns the method names that are not considered static or false if all
 *          methods are static
 */ export function hasNonStaticMethods(handlers) {
    if (// Order these by how common they are to be used
    handlers.POST || handlers.POST || handlers.DELETE || handlers.PATCH || handlers.OPTIONS) {
        return true;
    }
    return false;
}
// These symbols will be used to stash cached values on Proxied requests without requiring
// additional closures or storage such as WeakMaps.
const nextURLSymbol = Symbol("nextUrl");
const requestCloneSymbol = Symbol("clone");
const urlCloneSymbol = Symbol("clone");
const searchParamsSymbol = Symbol("searchParams");
const hrefSymbol = Symbol("href");
const toStringSymbol = Symbol("toString");
const headersSymbol = Symbol("headers");
const cookiesSymbol = Symbol("cookies");
/**
 * The general technique with these proxy handlers is to prioritize keeping them static
 * by stashing computed values on the Proxy itself. This is safe because the Proxy is
 * inaccessible to the consumer since all operations are forwarded
 */ const forceStaticRequestHandlers = {
    get (target, prop, receiver) {
        switch(prop){
            case "headers":
                return target[headersSymbol] || (target[headersSymbol] = HeadersAdapter.seal(new Headers({})));
            case "cookies":
                return target[cookiesSymbol] || (target[cookiesSymbol] = RequestCookiesAdapter.seal(new RequestCookies(new Headers({}))));
            case "nextUrl":
                return target[nextURLSymbol] || (target[nextURLSymbol] = new Proxy(target.nextUrl, forceStaticNextUrlHandlers));
            case "url":
                // we don't need to separately cache this we can just read the nextUrl
                // and return the href since we know it will have been stripped of any
                // dynamic parts. We access via the receiver to trigger the get trap
                return receiver.nextUrl.href;
            case "geo":
            case "ip":
                return undefined;
            case "clone":
                return target[requestCloneSymbol] || (target[requestCloneSymbol] = ()=>new Proxy(// This is vaguely unsafe but it's required since NextRequest does not implement
                    // clone. The reason we might expect this to work in this context is the Proxy will
                    // respond with static-amenable values anyway somewhat restoring the interface.
                    // @TODO we need to rethink NextRequest and NextURL because they are not sufficientlly
                    // sophisticated to adequately represent themselves in all contexts. A better approach is
                    // to probably embed the static generation logic into the class itself removing the need
                    // for any kind of proxying
                    target.clone(), forceStaticRequestHandlers));
            default:
                return ReflectAdapter.get(target, prop, receiver);
        }
    }
};
const forceStaticNextUrlHandlers = {
    get (target, prop, receiver) {
        switch(prop){
            // URL properties
            case "search":
                return "";
            case "searchParams":
                return target[searchParamsSymbol] || (target[searchParamsSymbol] = new URLSearchParams());
            case "href":
                return target[hrefSymbol] || (target[hrefSymbol] = cleanURL(target.href).href);
            case "toJSON":
            case "toString":
                return target[toStringSymbol] || (target[toStringSymbol] = ()=>receiver.href);
            // NextUrl properties
            case "url":
                // Currently nextURL does not expose url but our Docs indicate that it is an available property
                // I am forcing this to undefined here to avoid accidentally exposing a dynamic value later if
                // the underlying nextURL ends up adding this property
                return undefined;
            case "clone":
                return target[urlCloneSymbol] || (target[urlCloneSymbol] = ()=>new Proxy(target.clone(), forceStaticNextUrlHandlers));
            default:
                return ReflectAdapter.get(target, prop, receiver);
        }
    }
};
function proxyNextRequest(request, staticGenerationStore) {
    const nextUrlHandlers = {
        get (target, prop, receiver) {
            switch(prop){
                case "search":
                case "searchParams":
                case "url":
                case "href":
                case "toJSON":
                case "toString":
                case "origin":
                    {
                        trackDynamicDataAccessed(staticGenerationStore, `nextUrl.${prop}`);
                        return ReflectAdapter.get(target, prop, receiver);
                    }
                case "clone":
                    return target[urlCloneSymbol] || (target[urlCloneSymbol] = ()=>new Proxy(target.clone(), nextUrlHandlers));
                default:
                    return ReflectAdapter.get(target, prop, receiver);
            }
        }
    };
    const nextRequestHandlers = {
        get (target, prop) {
            switch(prop){
                case "nextUrl":
                    return target[nextURLSymbol] || (target[nextURLSymbol] = new Proxy(target.nextUrl, nextUrlHandlers));
                case "headers":
                case "cookies":
                case "url":
                case "body":
                case "blob":
                case "json":
                case "text":
                case "arrayBuffer":
                case "formData":
                    {
                        trackDynamicDataAccessed(staticGenerationStore, `request.${prop}`);
                        // The receiver arg is intentionally the same as the target to fix an issue with
                        // edge runtime, where attempting to access internal slots with the wrong `this` context
                        // results in an error.
                        return ReflectAdapter.get(target, prop, target);
                    }
                case "clone":
                    return target[requestCloneSymbol] || (target[requestCloneSymbol] = ()=>new Proxy(// This is vaguely unsafe but it's required since NextRequest does not implement
                        // clone. The reason we might expect this to work in this context is the Proxy will
                        // respond with static-amenable values anyway somewhat restoring the interface.
                        // @TODO we need to rethink NextRequest and NextURL because they are not sufficientlly
                        // sophisticated to adequately represent themselves in all contexts. A better approach is
                        // to probably embed the static generation logic into the class itself removing the need
                        // for any kind of proxying
                        target.clone(), nextRequestHandlers));
                default:
                    // The receiver arg is intentionally the same as the target to fix an issue with
                    // edge runtime, where attempting to access internal slots with the wrong `this` context
                    // results in an error.
                    return ReflectAdapter.get(target, prop, target);
            }
        }
    };
    return new Proxy(request, nextRequestHandlers);
}
const requireStaticRequestHandlers = {
    get (target, prop, receiver) {
        switch(prop){
            case "nextUrl":
                return target[nextURLSymbol] || (target[nextURLSymbol] = new Proxy(target.nextUrl, requireStaticNextUrlHandlers));
            case "headers":
            case "cookies":
            case "url":
            case "body":
            case "blob":
            case "json":
            case "text":
            case "arrayBuffer":
            case "formData":
                throw new StaticGenBailoutError(`Route ${target.nextUrl.pathname} with \`dynamic = "error"\` couldn't be rendered statically because it used \`request.${prop}\`.`);
            case "clone":
                return target[requestCloneSymbol] || (target[requestCloneSymbol] = ()=>new Proxy(// This is vaguely unsafe but it's required since NextRequest does not implement
                    // clone. The reason we might expect this to work in this context is the Proxy will
                    // respond with static-amenable values anyway somewhat restoring the interface.
                    // @TODO we need to rethink NextRequest and NextURL because they are not sufficientlly
                    // sophisticated to adequately represent themselves in all contexts. A better approach is
                    // to probably embed the static generation logic into the class itself removing the need
                    // for any kind of proxying
                    target.clone(), requireStaticRequestHandlers));
            default:
                return ReflectAdapter.get(target, prop, receiver);
        }
    }
};
const requireStaticNextUrlHandlers = {
    get (target, prop, receiver) {
        switch(prop){
            case "search":
            case "searchParams":
            case "url":
            case "href":
            case "toJSON":
            case "toString":
            case "origin":
                throw new StaticGenBailoutError(`Route ${target.pathname} with \`dynamic = "error"\` couldn't be rendered statically because it used \`nextUrl.${prop}\`.`);
            case "clone":
                return target[urlCloneSymbol] || (target[urlCloneSymbol] = ()=>new Proxy(target.clone(), requireStaticNextUrlHandlers));
            default:
                return ReflectAdapter.get(target, prop, receiver);
        }
    }
};

//# sourceMappingURL=module.js.map