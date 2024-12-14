"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "makeGetServerInsertedHTML", {
    enumerable: true,
    get: function() {
        return makeGetServerInsertedHTML;
    }
});
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_default(require("react"));
const _httpaccessfallback = require("../../client/components/http-access-fallback/http-access-fallback");
const _redirect = require("../../client/components/redirect");
const _redirecterror = require("../../client/components/redirect-error");
const _serveredge = require("react-dom/server.edge");
const _nodewebstreamshelper = require("../stream-utils/node-web-streams-helper");
const _redirectstatuscode = require("../../client/components/redirect-status-code");
const _addpathprefix = require("../../shared/lib/router/utils/add-path-prefix");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function makeGetServerInsertedHTML({ polyfills, renderServerInsertedHTML, serverCapturedErrors, tracingMetadata, basePath }) {
    let flushedErrorMetaTagsUntilIndex = 0;
    // flag for static content that only needs to be flushed once
    let hasFlushedInitially = false;
    const polyfillTags = polyfills.map((polyfill)=>{
        return /*#__PURE__*/ (0, _jsxruntime.jsx)("script", {
            ...polyfill
        }, polyfill.src);
    });
    return async function getServerInsertedHTML() {
        // Loop through all the errors that have been captured but not yet
        // flushed.
        const errorMetaTags = [];
        while(flushedErrorMetaTagsUntilIndex < serverCapturedErrors.length){
            const error = serverCapturedErrors[flushedErrorMetaTagsUntilIndex];
            flushedErrorMetaTagsUntilIndex++;
            if ((0, _httpaccessfallback.isHTTPAccessFallbackError)(error)) {
                errorMetaTags.push(/*#__PURE__*/ (0, _jsxruntime.jsx)("meta", {
                    name: "robots",
                    content: "noindex"
                }, error.digest), process.env.NODE_ENV === 'development' ? /*#__PURE__*/ (0, _jsxruntime.jsx)("meta", {
                    name: "next-error",
                    content: "not-found"
                }, "next-error") : null);
            } else if ((0, _redirecterror.isRedirectError)(error)) {
                const redirectUrl = (0, _addpathprefix.addPathPrefix)((0, _redirect.getURLFromRedirectError)(error), basePath);
                const statusCode = (0, _redirect.getRedirectStatusCodeFromError)(error);
                const isPermanent = statusCode === _redirectstatuscode.RedirectStatusCode.PermanentRedirect ? true : false;
                if (redirectUrl) {
                    errorMetaTags.push(/*#__PURE__*/ (0, _jsxruntime.jsx)("meta", {
                        id: "__next-page-redirect",
                        httpEquiv: "refresh",
                        content: `${isPermanent ? 0 : 1};url=${redirectUrl}`
                    }, error.digest));
                }
            }
        }
        const traceMetaTags = (tracingMetadata || []).map(({ key, value }, index)=>/*#__PURE__*/ (0, _jsxruntime.jsx)("meta", {
                name: key,
                content: value
            }, `next-trace-data-${index}`));
        const serverInsertedHTML = renderServerInsertedHTML();
        // Skip React rendering if we know the content is empty.
        if (polyfillTags.length === 0 && traceMetaTags.length === 0 && errorMetaTags.length === 0 && Array.isArray(serverInsertedHTML) && serverInsertedHTML.length === 0) {
            return '';
        }
        const stream = await (0, _serveredge.renderToReadableStream)(/*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
            children: [
                /* Insert the polyfills if they haven't been flushed yet. */ hasFlushedInitially ? null : polyfillTags,
                serverInsertedHTML,
                hasFlushedInitially ? null : traceMetaTags,
                errorMetaTags
            ]
        }), {
            // Larger chunk because this isn't sent over the network.
            // Let's set it to 1MB.
            progressiveChunkSize: 1024 * 1024
        });
        hasFlushedInitially = true;
        // There's no need to wait for the stream to be ready
        // e.g. calling `await stream.allReady` because `streamToString` will
        // wait and decode the stream progressively with better parallelism.
        return (0, _nodewebstreamshelper.streamToString)(stream);
    };
}

//# sourceMappingURL=make-get-server-inserted-html.js.map