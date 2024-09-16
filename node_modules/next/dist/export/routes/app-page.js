"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    ExportedAppPageFiles: null,
    exportAppPage: null
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
    }
});
const _isdynamicusageerror = require("../helpers/is-dynamic-usage-error");
const _constants = require("../../lib/constants");
const _ciinfo = require("../../telemetry/ci-info");
const _modulerender = require("../../server/future/route-modules/app-page/module.render");
const _bailouttocsr = require("../../shared/lib/lazy-dynamic/bailout-to-csr");
var ExportedAppPageFiles;
(function(ExportedAppPageFiles) {
    ExportedAppPageFiles["HTML"] = "HTML";
    ExportedAppPageFiles["FLIGHT"] = "FLIGHT";
    ExportedAppPageFiles["PREFETCH_FLIGHT"] = "PREFETCH_FLIGHT";
    ExportedAppPageFiles["META"] = "META";
    ExportedAppPageFiles["POSTPONED"] = "POSTPONED";
})(ExportedAppPageFiles || (ExportedAppPageFiles = {}));
async function exportAppPage(req, res, page, path, pathname, query, renderOpts, htmlFilepath, debugOutput, isDynamicError, fileWriter) {
    let isDefaultNotFound = false;
    // If the page is `/_not-found`, then we should update the page to be `/404`.
    // UNDERSCORE_NOT_FOUND_ROUTE value used here, however we don't want to import it here as it causes constants to be inlined which we don't want here.
    if (page === "/_not-found/page") {
        isDefaultNotFound = true;
        pathname = "/404";
    }
    try {
        const result = await (0, _modulerender.lazyRenderAppPage)(req, res, pathname, query, renderOpts);
        const html = result.toUnchunkedString();
        const { metadata } = result;
        const { flightData, revalidate = false, postponed, fetchTags } = metadata;
        // Ensure we don't postpone without having PPR enabled.
        if (postponed && !renderOpts.experimental.ppr) {
            throw new Error("Invariant: page postponed without PPR being enabled");
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
                revalidate: 0
            };
        } else if (!flightData) {
            throw new Error(`Invariant: failed to get page data for ${path}`);
        } else if (renderOpts.experimental.ppr) {
            // If PPR is enabled, we should emit the flight data as the prefetch
            // payload.
            await fileWriter("PREFETCH_FLIGHT", htmlFilepath.replace(/\.html$/, _constants.RSC_PREFETCH_SUFFIX), flightData);
        } else {
            // Writing the RSC payload to a file if we don't have PPR enabled.
            await fileWriter("FLIGHT", htmlFilepath.replace(/\.html$/, _constants.RSC_SUFFIX), flightData);
        }
        const headers = {
            ...metadata.headers
        };
        // When PPR is enabled, we should grab the headers from the mocked response
        // and add it to the headers.
        if (renderOpts.experimental.ppr) {
            Object.assign(headers, res.getHeaders());
        }
        if (fetchTags) {
            headers[_constants.NEXT_CACHE_TAGS_HEADER] = fetchTags;
        }
        // Writing static HTML to a file.
        await fileWriter("HTML", htmlFilepath, html ?? "", "utf8");
        const isParallelRoute = /\/@\w+/.test(page);
        const isNonSuccessfulStatusCode = res.statusCode > 300;
        // When PPR is enabled, we don't always send 200 for routes that have been
        // pregenerated, so we should grab the status code from the mocked
        // response.
        let status = renderOpts.experimental.ppr ? res.statusCode : undefined;
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
            postponed
        };
        await fileWriter("META", htmlFilepath.replace(/\.html$/, _constants.NEXT_META_SUFFIX), JSON.stringify(meta, null, 2));
        return {
            // Only include the metadata if the environment has next support.
            metadata: _ciinfo.hasNextSupport ? meta : undefined,
            hasEmptyPrelude: Boolean(postponed) && html === "",
            hasPostponed: Boolean(postponed),
            revalidate
        };
    } catch (err) {
        if (!(0, _isdynamicusageerror.isDynamicUsageError)(err)) {
            throw err;
        }
        // If enabled, we should fail rendering if a client side rendering bailout
        // occurred at the page level.
        if (renderOpts.experimental.missingSuspenseWithCSRBailout && (0, _bailouttocsr.isBailoutToCSRError)(err)) {
            throw err;
        }
        if (debugOutput) {
            const { dynamicUsageDescription, dynamicUsageStack } = renderOpts.store;
            logDynamicUsageWarning({
                path,
                description: dynamicUsageDescription,
                stack: dynamicUsageStack
            });
        }
        return {
            revalidate: 0
        };
    }
}
function logDynamicUsageWarning({ path, description, stack }) {
    const errMessage = new Error(`Static generation failed due to dynamic usage on ${path}, reason: ${description}`);
    if (stack) {
        errMessage.stack = errMessage.message + stack.substring(stack.indexOf("\n"));
    }
    console.warn(errMessage);
}

//# sourceMappingURL=app-page.js.map