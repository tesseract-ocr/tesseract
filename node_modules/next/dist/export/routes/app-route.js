"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    ExportedAppRouteFiles: null,
    exportAppRoute: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    ExportedAppRouteFiles: function() {
        return ExportedAppRouteFiles;
    },
    exportAppRoute: function() {
        return exportAppRoute;
    }
});
const _path = require("path");
const _constants = require("../../lib/constants");
const _node = require("../../server/base-http/node");
const _routemoduleloader = require("../../server/future/helpers/module-loader/route-module-loader");
const _nextrequest = require("../../server/web/spec-extension/adapters/next-request");
const _utils = require("../../server/web/utils");
const _isdynamicusageerror = require("../helpers/is-dynamic-usage-error");
const _constants1 = require("../../shared/lib/constants");
const _ciinfo = require("../../telemetry/ci-info");
var ExportedAppRouteFiles;
(function(ExportedAppRouteFiles) {
    ExportedAppRouteFiles["BODY"] = "BODY";
    ExportedAppRouteFiles["META"] = "META";
})(ExportedAppRouteFiles || (ExportedAppRouteFiles = {}));
async function exportAppRoute(req, res, params, page, incrementalCache, distDir, htmlFilepath, fileWriter) {
    // Ensure that the URL is absolute.
    req.url = `http://localhost:3000${req.url}`;
    // Adapt the request and response to the Next.js request and response.
    const request = _nextrequest.NextRequestAdapter.fromNodeNextRequest(new _node.NodeNextRequest(req), (0, _nextrequest.signalFromNodeResponse)(res));
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
    if (_ciinfo.hasNextSupport) {
        context.renderOpts.isRevalidate = true;
    }
    // This is a route handler, which means it has it's handler in the
    // bundled file already, we should just use that.
    const filename = (0, _path.join)(distDir, _constants1.SERVER_DIRECTORY, "app", page);
    try {
        var _context_renderOpts_store;
        // Route module loading and handling.
        const module1 = await _routemoduleloader.RouteModuleLoader.load(filename);
        const response = await module1.handle(request, context);
        const isValidStatus = response.status < 400 || response.status === 404;
        if (!isValidStatus) {
            return {
                revalidate: 0
            };
        }
        const blob = await response.blob();
        const revalidate = typeof ((_context_renderOpts_store = context.renderOpts.store) == null ? void 0 : _context_renderOpts_store.revalidate) === "undefined" ? false : context.renderOpts.store.revalidate;
        const headers = (0, _utils.toNodeOutgoingHttpHeaders)(response.headers);
        const cacheTags = context.renderOpts.fetchTags;
        if (cacheTags) {
            headers[_constants.NEXT_CACHE_TAGS_HEADER] = cacheTags;
        }
        if (!headers["content-type"] && blob.type) {
            headers["content-type"] = blob.type;
        }
        // Writing response body to a file.
        const body = Buffer.from(await blob.arrayBuffer());
        await fileWriter("BODY", htmlFilepath.replace(/\.html$/, _constants.NEXT_BODY_SUFFIX), body, "utf8");
        // Write the request metadata to a file.
        const meta = {
            status: response.status,
            headers
        };
        await fileWriter("META", htmlFilepath.replace(/\.html$/, _constants.NEXT_META_SUFFIX), JSON.stringify(meta));
        return {
            revalidate: revalidate,
            metadata: meta
        };
    } catch (err) {
        if (!(0, _isdynamicusageerror.isDynamicUsageError)(err)) {
            throw err;
        }
        return {
            revalidate: 0
        };
    }
}

//# sourceMappingURL=app-route.js.map