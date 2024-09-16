/**
 * Contains predefined constants for the trace span name in next/server.
 *
 * Currently, next/server/tracer is internal implementation only for tracking
 * next.js's implementation only with known span names defined here.
 **/
declare enum BaseServerSpan {
    handleRequest = "BaseServer.handleRequest",
    run = "BaseServer.run",
    pipe = "BaseServer.pipe",
    getStaticHTML = "BaseServer.getStaticHTML",
    render = "BaseServer.render",
    renderToResponseWithComponents = "BaseServer.renderToResponseWithComponents",
    renderToResponse = "BaseServer.renderToResponse",
    renderToHTML = "BaseServer.renderToHTML",
    renderError = "BaseServer.renderError",
    renderErrorToResponse = "BaseServer.renderErrorToResponse",
    renderErrorToHTML = "BaseServer.renderErrorToHTML",
    render404 = "BaseServer.render404"
}
declare enum LoadComponentsSpan {
    loadDefaultErrorComponents = "LoadComponents.loadDefaultErrorComponents",
    loadComponents = "LoadComponents.loadComponents"
}
declare enum NextServerSpan {
    getRequestHandler = "NextServer.getRequestHandler",
    getServer = "NextServer.getServer",
    getServerRequestHandler = "NextServer.getServerRequestHandler",
    createServer = "createServer.createServer"
}
declare enum NextNodeServerSpan {
    compression = "NextNodeServer.compression",
    getBuildId = "NextNodeServer.getBuildId",
    createComponentTree = "NextNodeServer.createComponentTree",
    clientComponentLoading = "NextNodeServer.clientComponentLoading",
    getLayoutOrPageModule = "NextNodeServer.getLayoutOrPageModule",
    generateStaticRoutes = "NextNodeServer.generateStaticRoutes",
    generateFsStaticRoutes = "NextNodeServer.generateFsStaticRoutes",
    generatePublicRoutes = "NextNodeServer.generatePublicRoutes",
    generateImageRoutes = "NextNodeServer.generateImageRoutes.route",
    sendRenderResult = "NextNodeServer.sendRenderResult",
    proxyRequest = "NextNodeServer.proxyRequest",
    runApi = "NextNodeServer.runApi",
    render = "NextNodeServer.render",
    renderHTML = "NextNodeServer.renderHTML",
    imageOptimizer = "NextNodeServer.imageOptimizer",
    getPagePath = "NextNodeServer.getPagePath",
    getRoutesManifest = "NextNodeServer.getRoutesManifest",
    findPageComponents = "NextNodeServer.findPageComponents",
    getFontManifest = "NextNodeServer.getFontManifest",
    getServerComponentManifest = "NextNodeServer.getServerComponentManifest",
    getRequestHandler = "NextNodeServer.getRequestHandler",
    renderToHTML = "NextNodeServer.renderToHTML",
    renderError = "NextNodeServer.renderError",
    renderErrorToHTML = "NextNodeServer.renderErrorToHTML",
    render404 = "NextNodeServer.render404",
    startResponse = "NextNodeServer.startResponse",
    route = "route",
    onProxyReq = "onProxyReq",
    apiResolver = "apiResolver",
    internalFetch = "internalFetch"
}
declare enum StartServerSpan {
    startServer = "startServer.startServer"
}
declare enum RenderSpan {
    getServerSideProps = "Render.getServerSideProps",
    getStaticProps = "Render.getStaticProps",
    renderToString = "Render.renderToString",
    renderDocument = "Render.renderDocument",
    createBodyResult = "Render.createBodyResult"
}
declare enum AppRenderSpan {
    renderToString = "AppRender.renderToString",
    renderToReadableStream = "AppRender.renderToReadableStream",
    getBodyResult = "AppRender.getBodyResult",
    fetch = "AppRender.fetch"
}
declare enum RouterSpan {
    executeRoute = "Router.executeRoute"
}
declare enum NodeSpan {
    runHandler = "Node.runHandler"
}
declare enum AppRouteRouteHandlersSpan {
    runHandler = "AppRouteRouteHandlers.runHandler"
}
declare enum ResolveMetadataSpan {
    generateMetadata = "ResolveMetadata.generateMetadata",
    generateViewport = "ResolveMetadata.generateViewport"
}
declare enum MiddlewareSpan {
    execute = "Middleware.execute"
}
type SpanTypes = `${BaseServerSpan}` | `${LoadComponentsSpan}` | `${NextServerSpan}` | `${StartServerSpan}` | `${NextNodeServerSpan}` | `${RenderSpan}` | `${RouterSpan}` | `${AppRenderSpan}` | `${NodeSpan}` | `${AppRouteRouteHandlersSpan}` | `${ResolveMetadataSpan}` | `${MiddlewareSpan}`;
export declare const NextVanillaSpanAllowlist: (BaseServerSpan | NextNodeServerSpan | RenderSpan | AppRenderSpan | NodeSpan | AppRouteRouteHandlersSpan | ResolveMetadataSpan | MiddlewareSpan)[];
export declare const LogSpanAllowList: NextNodeServerSpan[];
export { BaseServerSpan, LoadComponentsSpan, NextServerSpan, NextNodeServerSpan, StartServerSpan, RenderSpan, RouterSpan, AppRenderSpan, NodeSpan, AppRouteRouteHandlersSpan, ResolveMetadataSpan, MiddlewareSpan, };
export type { SpanTypes };
