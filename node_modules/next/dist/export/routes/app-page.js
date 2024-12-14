"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    ExportedAppPageFiles: null,
    exportAppPage: null,
    prospectiveRenderAppPage: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    ExportedAppPageFiles: function() {
        return ExportedAppPageFiles;
    },
    exportAppPage: function() {
        return exportAppPage;
    },
    prospectiveRenderAppPage: function() {
        return prospectiveRenderAppPage;
    }
});
const _isdynamicusageerror = require("../helpers/is-dynamic-usage-error");
const _constants = require("../../lib/constants");
const _ciinfo = require("../../server/ci-info");
const _modulerender = require("../../server/route-modules/app-page/module.render");
const _bailouttocsr = require("../../shared/lib/lazy-dynamic/bailout-to-csr");
const _node = require("../../server/base-http/node");
const _approuterheaders = require("../../client/components/app-router-headers");
const _runwithafter = require("../../server/after/run-with-after");
var ExportedAppPageFiles = /*#__PURE__*/ function(ExportedAppPageFiles) {
    ExportedAppPageFiles["HTML"] = "HTML";
    ExportedAppPageFiles["FLIGHT"] = "FLIGHT";
    ExportedAppPageFiles["PREFETCH_FLIGHT"] = "PREFETCH_FLIGHT";
    ExportedAppPageFiles["PREFETCH_FLIGHT_SEGMENT"] = "PREFETCH_FLIGHT_SEGMENT";
    ExportedAppPageFiles["META"] = "META";
    ExportedAppPageFiles["POSTPONED"] = "POSTPONED";
    return ExportedAppPageFiles;
}({});
async function prospectiveRenderAppPage(req, res, page, pathname, query, fallbackRouteParams, partialRenderOpts) {
    const afterRunner = new _runwithafter.AfterRunner();
    // If the page is `/_not-found`, then we should update the page to be `/404`.
    // UNDERSCORE_NOT_FOUND_ROUTE value used here, however we don't want to import it here as it causes constants to be inlined which we don't want here.
    if (page === '/_not-found/page') {
        pathname = '/404';
    }
    try {
        await (0, _modulerender.lazyRenderAppPage)(new _node.NodeNextRequest(req), new _node.NodeNextResponse(res), pathname, query, fallbackRouteParams, {
            ...partialRenderOpts,
            waitUntil: afterRunner.context.waitUntil,
            onClose: afterRunner.context.onClose,
            onAfterTaskError: afterRunner.context.onTaskError
        }, undefined, false);
        // TODO(after): if we abort a prerender because of an error in an after-callback
        // we should probably communicate that better (and not log the error twice)
        await afterRunner.executeAfter();
    } catch (err) {
        if (!(0, _isdynamicusageerror.isDynamicUsageError)(err)) {
            throw err;
        }
        // We should fail rendering if a client side rendering bailout
        // occurred at the page level.
        if ((0, _bailouttocsr.isBailoutToCSRError)(err)) {
            throw err;
        }
    }
}
async function exportAppPage(req, res, page, path, pathname, query, fallbackRouteParams, partialRenderOpts, htmlFilepath, debugOutput, isDynamicError, fileWriter) {
    const afterRunner = new _runwithafter.AfterRunner();
    const renderOpts = {
        ...partialRenderOpts,
        waitUntil: afterRunner.context.waitUntil,
        onClose: afterRunner.context.onClose,
        onAfterTaskError: afterRunner.context.onTaskError
    };
    let isDefaultNotFound = false;
    // If the page is `/_not-found`, then we should update the page to be `/404`.
    // UNDERSCORE_NOT_FOUND_ROUTE value used here, however we don't want to import it here as it causes constants to be inlined which we don't want here.
    if (page === '/_not-found/page') {
        isDefaultNotFound = true;
        pathname = '/404';
    }
    try {
        const result = await (0, _modulerender.lazyRenderAppPage)(new _node.NodeNextRequest(req), new _node.NodeNextResponse(res), pathname, query, fallbackRouteParams, renderOpts, undefined, false);
        const html = result.toUnchunkedString();
        // TODO(after): if we abort a prerender because of an error in an after-callback
        // we should probably communicate that better (and not log the error twice)
        await afterRunner.executeAfter();
        const { metadata } = result;
        const { flightData, revalidate = false, postponed, fetchTags, fetchMetrics, segmentData } = metadata;
        // Ensure we don't postpone without having PPR enabled.
        if (postponed && !renderOpts.experimental.isRoutePPREnabled) {
            throw new Error('Invariant: page postponed without PPR being enabled');
        }
        if (revalidate === 0) {
            if (isDynamicError) {
                throw new Error(`Page with dynamic = "error" encountered dynamic data method on ${path}.`);
            }
            const { staticBailoutInfo = {} } = metadata;
            if (revalidate === 0 && debugOutput && (staticBailoutInfo == null ? void 0 : staticBailoutInfo.description)) {
                logDynamicUsageWarning({
                    path,
                    description: staticBailoutInfo.description,
                    stack: staticBailoutInfo.stack
                });
            }
            return {
                revalidate: 0,
                fetchMetrics
            };
        }
        // If page data isn't available, it means that the page couldn't be rendered
        // properly so long as we don't have unknown route params. When a route doesn't
        // have unknown route params, there will not be any flight data.
        if (!flightData && (!fallbackRouteParams || fallbackRouteParams.size === 0)) {
            throw new Error(`Invariant: failed to get page data for ${path}`);
        }
        let segmentPaths;
        if (flightData) {
            // If PPR is enabled, we want to emit a prefetch rsc file for the page
            // instead of the standard rsc. This is because the standard rsc will
            // contain the dynamic data. We do this if any routes have PPR enabled so
            // that the cache read/write is the same.
            if (renderOpts.experimental.isRoutePPREnabled) {
                // If PPR is enabled, we should emit the flight data as the prefetch
                // payload.
                // TODO: This will eventually be replaced by the per-segment prefetch
                // output below.
                await fileWriter("PREFETCH_FLIGHT", htmlFilepath.replace(/\.html$/, _constants.RSC_PREFETCH_SUFFIX), flightData);
                if (segmentData) {
                    // Emit the per-segment prefetch data. We emit them as separate files
                    // so that the cache handler has the option to treat each as a
                    // separate entry.
                    segmentPaths = [];
                    const segmentsDir = htmlFilepath.replace(/\.html$/, _constants.RSC_SEGMENTS_DIR_SUFFIX);
                    const tasks = [];
                    for (const [segmentPath, buffer] of segmentData){
                        segmentPaths.push(segmentPath);
                        const segmentDataFilePath = segmentPath === '/' ? segmentsDir + '/_index' + _constants.RSC_SEGMENT_SUFFIX : segmentsDir + segmentPath + _constants.RSC_SEGMENT_SUFFIX;
                        tasks.push(fileWriter("PREFETCH_FLIGHT_SEGMENT", segmentDataFilePath, buffer));
                    }
                    await Promise.all(tasks);
                }
            } else {
                // Writing the RSC payload to a file if we don't have PPR enabled.
                await fileWriter("FLIGHT", htmlFilepath.replace(/\.html$/, _constants.RSC_SUFFIX), flightData);
            }
        }
        const headers = {
            ...metadata.headers
        };
        // If we're writing the file to disk, we know it's a prerender.
        headers[_approuterheaders.NEXT_IS_PRERENDER_HEADER] = '1';
        if (fetchTags) {
            headers[_constants.NEXT_CACHE_TAGS_HEADER] = fetchTags;
        }
        // Writing static HTML to a file.
        await fileWriter("HTML", htmlFilepath, html ?? '', 'utf8');
        const isParallelRoute = /\/@\w+/.test(page);
        const isNonSuccessfulStatusCode = res.statusCode > 300;
        // When PPR is enabled, we don't always send 200 for routes that have been
        // pregenerated, so we should grab the status code from the mocked
        // response.
        let status = renderOpts.experimental.isRoutePPREnabled ? res.statusCode : undefined;
        if (isDefaultNotFound) {
            // Override the default /_not-found page status code to 404
            status = 404;
        } else if (isNonSuccessfulStatusCode && !isParallelRoute) {
            // If it's parallel route the status from mock response is 404
            status = res.statusCode;
        }
        // Writing the request metadata to a file.
        const meta = {
            status,
            headers,
            postponed,
            segmentPaths
        };
        await fileWriter("META", htmlFilepath.replace(/\.html$/, _constants.NEXT_META_SUFFIX), JSON.stringify(meta, null, 2));
        return {
            // Only include the metadata if the environment has next support.
            metadata: _ciinfo.hasNextSupport ? meta : undefined,
            hasEmptyPrelude: Boolean(postponed) && html === '',
            hasPostponed: Boolean(postponed),
            revalidate,
            fetchMetrics
        };
    } catch (err) {
        if (!(0, _isdynamicusageerror.isDynamicUsageError)(err)) {
            throw err;
        }
        // We should fail rendering if a client side rendering bailout
        // occurred at the page level.
        if ((0, _bailouttocsr.isBailoutToCSRError)(err)) {
            throw err;
        }
        let fetchMetrics;
        if (debugOutput) {
            const store = renderOpts.store;
            const { dynamicUsageDescription, dynamicUsageStack } = store;
            fetchMetrics = store.fetchMetrics;
            logDynamicUsageWarning({
                path,
                description: dynamicUsageDescription ?? '',
                stack: dynamicUsageStack
            });
        }
        return {
            revalidate: 0,
            fetchMetrics
        };
    }
}
function logDynamicUsageWarning({ path, description, stack }) {
    const errMessage = new Error(`Static generation failed due to dynamic usage on ${path}, reason: ${description}`);
    if (stack) {
        errMessage.stack = errMessage.message + stack.substring(stack.indexOf('\n'));
    }
    console.warn(errMessage);
}

//# sourceMappingURL=app-page.js.map