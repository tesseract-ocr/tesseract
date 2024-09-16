import { join } from "path";
import { NEXT_BODY_SUFFIX, NEXT_CACHE_TAGS_HEADER, NEXT_META_SUFFIX } from "../../lib/constants";
import { NodeNextRequest } from "../../server/base-http/node";
import { RouteModuleLoader } from "../../server/future/helpers/module-loader/route-module-loader";
import { NextRequestAdapter, signalFromNodeResponse } from "../../server/web/spec-extension/adapters/next-request";
import { toNodeOutgoingHttpHeaders } from "../../server/web/utils";
import { isDynamicUsageError } from "../helpers/is-dynamic-usage-error";
import { SERVER_DIRECTORY } from "../../shared/lib/constants";
import { hasNextSupport } from "../../telemetry/ci-info";
export var ExportedAppRouteFiles;
(function(ExportedAppRouteFiles) {
    ExportedAppRouteFiles["BODY"] = "BODY";
    ExportedAppRouteFiles["META"] = "META";
})(ExportedAppRouteFiles || (ExportedAppRouteFiles = {}));
export async function exportAppRoute(req, res, params, page, incrementalCache, distDir, htmlFilepath, fileWriter) {
    // Ensure that the URL is absolute.
    req.url = `http://localhost:3000${req.url}`;
    // Adapt the request and response to the Next.js request and response.
    const request = NextRequestAdapter.fromNodeNextRequest(new NodeNextRequest(req), signalFromNodeResponse(res));
    // Create the context for the handler. This contains the params from
    // the route and the context for the request.
    const context = {
        params,
        prerenderManifest: {
            version: 4,
            routes: {},
            dynamicRoutes: {},
            preview: {
                previewModeEncryptionKey: "",
                previewModeId: "",
                previewModeSigningKey: ""
            },
            notFoundRoutes: []
        },
        renderOpts: {
            experimental: {
                ppr: false
            },
            originalPathname: page,
            nextExport: true,
            supportsDynamicHTML: false,
            incrementalCache
        }
    };
    if (hasNextSupport) {
        context.renderOpts.isRevalidate = true;
    }
    // This is a route handler, which means it has it's handler in the
    // bundled file already, we should just use that.
    const filename = join(distDir, SERVER_DIRECTORY, "app", page);
    try {
        var _context_renderOpts_store;
        // Route module loading and handling.
        const module = await RouteModuleLoader.load(filename);
        const response = await module.handle(request, context);
        const isValidStatus = response.status < 400 || response.status === 404;
        if (!isValidStatus) {
            return {
                revalidate: 0
            };
        }
        const blob = await response.blob();
        const revalidate = typeof ((_context_renderOpts_store = context.renderOpts.store) == null ? void 0 : _context_renderOpts_store.revalidate) === "undefined" ? false : context.renderOpts.store.revalidate;
        const headers = toNodeOutgoingHttpHeaders(response.headers);
        const cacheTags = context.renderOpts.fetchTags;
        if (cacheTags) {
            headers[NEXT_CACHE_TAGS_HEADER] = cacheTags;
        }
        if (!headers["content-type"] && blob.type) {
            headers["content-type"] = blob.type;
        }
        // Writing response body to a file.
        const body = Buffer.from(await blob.arrayBuffer());
        await fileWriter("BODY", htmlFilepath.replace(/\.html$/, NEXT_BODY_SUFFIX), body, "utf8");
        // Write the request metadata to a file.
        const meta = {
            status: response.status,
            headers
        };
        await fileWriter("META", htmlFilepath.replace(/\.html$/, NEXT_META_SUFFIX), JSON.stringify(meta));
        return {
            revalidate: revalidate,
            metadata: meta
        };
    } catch (err) {
        if (!isDynamicUsageError(err)) {
            throw err;
        }
        return {
            revalidate: 0
        };
    }
}

//# sourceMappingURL=app-route.js.map