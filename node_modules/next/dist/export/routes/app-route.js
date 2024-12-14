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
const _constants = require("../../lib/constants");
const _node = require("../../server/base-http/node");
const _nextrequest = require("../../server/web/spec-extension/adapters/next-request");
const _utils = require("../../server/web/utils");
const _isdynamicusageerror = require("../helpers/is-dynamic-usage-error");
const _ciinfo = require("../../server/ci-info");
const _isstaticgenenabled = require("../../server/route-modules/app-route/helpers/is-static-gen-enabled");
const _ismetadataroute = require("../../lib/metadata/is-metadata-route");
const _apppaths = require("../../shared/lib/router/utils/app-paths");
const _runwithafter = require("../../server/after/run-with-after");
var ExportedAppRouteFiles = /*#__PURE__*/ function(ExportedAppRouteFiles) {
    ExportedAppRouteFiles["BODY"] = "BODY";
    ExportedAppRouteFiles["META"] = "META";
    return ExportedAppRouteFiles;
}({});
async function exportAppRoute(req, res, params, page, module1, incrementalCache, cacheLifeProfiles, htmlFilepath, fileWriter, experimental, buildId) {
    // Ensure that the URL is absolute.
    req.url = `http://localhost:3000${req.url}`;
    // Adapt the request and response to the Next.js request and response.
    const request = _nextrequest.NextRequestAdapter.fromNodeNextRequest(new _node.NodeNextRequest(req), (0, _nextrequest.signalFromNodeResponse)(res));
    const afterRunner = new _runwithafter.AfterRunner();
    // Create the context for the handler. This contains the params from
    // the route and the context for the request.
    const context = {
        params,
        prerenderManifest: {
            version: 4,
            routes: {},
            dynamicRoutes: {},
            preview: {
                previewModeEncryptionKey: '',
                previewModeId: '',
                previewModeSigningKey: ''
            },
            notFoundRoutes: []
        },
        renderOpts: {
            experimental,
            nextExport: true,
            supportsDynamicResponse: false,
            incrementalCache,
            waitUntil: afterRunner.context.waitUntil,
            onClose: afterRunner.context.onClose,
            onAfterTaskError: afterRunner.context.onTaskError,
            cacheLifeProfiles,
            buildId
        }
    };
    if (_ciinfo.hasNextSupport) {
        context.renderOpts.isRevalidate = true;
    }
    try {
        const userland = module1.userland;
        // we don't bail from the static optimization for
        // metadata routes
        const normalizedPage = (0, _apppaths.normalizeAppPath)(page);
        const isMetadataRoute = (0, _ismetadataroute.isMetadataRouteFile)(normalizedPage, [], false);
        if (!(0, _isstaticgenenabled.isStaticGenEnabled)(userland) && !isMetadataRoute && // We don't disable static gen when dynamicIO is enabled because we
        // expect that anything dynamic in the GET handler will make it dynamic
        // and thus avoid the cache surprises that led to us removing static gen
        // unless specifically opted into
        experimental.dynamicIO !== true) {
            return {
                revalidate: 0
            };
        }
        const response = await module1.handle(request, context);
        const isValidStatus = response.status < 400 || response.status === 404;
        if (!isValidStatus) {
            return {
                revalidate: 0
            };
        }
        const blob = await response.blob();
        // TODO(after): if we abort a prerender because of an error in an after-callback
        // we should probably communicate that better (and not log the error twice)
        await afterRunner.executeAfter();
        const revalidate = typeof context.renderOpts.collectedRevalidate === 'undefined' || context.renderOpts.collectedRevalidate >= _constants.INFINITE_CACHE ? false : context.renderOpts.collectedRevalidate;
        const headers = (0, _utils.toNodeOutgoingHttpHeaders)(response.headers);
        const cacheTags = context.renderOpts.collectedTags;
        if (cacheTags) {
            headers[_constants.NEXT_CACHE_TAGS_HEADER] = cacheTags;
        }
        if (!headers['content-type'] && blob.type) {
            headers['content-type'] = blob.type;
        }
        // Writing response body to a file.
        const body = Buffer.from(await blob.arrayBuffer());
        await fileWriter("BODY", htmlFilepath.replace(/\.html$/, _constants.NEXT_BODY_SUFFIX), body, 'utf8');
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