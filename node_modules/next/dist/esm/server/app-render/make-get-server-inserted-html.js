import { jsx as _jsx, jsxs as _jsxs, Fragment as _Fragment } from "react/jsx-runtime";
import React from 'react';
import { isHTTPAccessFallbackError } from '../../client/components/http-access-fallback/http-access-fallback';
import { getURLFromRedirectError, getRedirectStatusCodeFromError } from '../../client/components/redirect';
import { isRedirectError } from '../../client/components/redirect-error';
import { renderToReadableStream } from 'react-dom/server.edge';
import { streamToString } from '../stream-utils/node-web-streams-helper';
import { RedirectStatusCode } from '../../client/components/redirect-status-code';
import { addPathPrefix } from '../../shared/lib/router/utils/add-path-prefix';
export function makeGetServerInsertedHTML({ polyfills, renderServerInsertedHTML, serverCapturedErrors, tracingMetadata, basePath }) {
    let flushedErrorMetaTagsUntilIndex = 0;
    // flag for static content that only needs to be flushed once
    let hasFlushedInitially = false;
    const polyfillTags = polyfills.map((polyfill)=>{
        return /*#__PURE__*/ _jsx("script", {
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
            if (isHTTPAccessFallbackError(error)) {
                errorMetaTags.push(/*#__PURE__*/ _jsx("meta", {
                    name: "robots",
                    content: "noindex"
                }, error.digest), process.env.NODE_ENV === 'development' ? /*#__PURE__*/ _jsx("meta", {
                    name: "next-error",
                    content: "not-found"
                }, "next-error") : null);
            } else if (isRedirectError(error)) {
                const redirectUrl = addPathPrefix(getURLFromRedirectError(error), basePath);
                const statusCode = getRedirectStatusCodeFromError(error);
                const isPermanent = statusCode === RedirectStatusCode.PermanentRedirect ? true : false;
                if (redirectUrl) {
                    errorMetaTags.push(/*#__PURE__*/ _jsx("meta", {
                        id: "__next-page-redirect",
                        httpEquiv: "refresh",
                        content: `${isPermanent ? 0 : 1};url=${redirectUrl}`
                    }, error.digest));
                }
            }
        }
        const traceMetaTags = (tracingMetadata || []).map(({ key, value }, index)=>/*#__PURE__*/ _jsx("meta", {
                name: key,
                content: value
            }, `next-trace-data-${index}`));
        const serverInsertedHTML = renderServerInsertedHTML();
        // Skip React rendering if we know the content is empty.
        if (polyfillTags.length === 0 && traceMetaTags.length === 0 && errorMetaTags.length === 0 && Array.isArray(serverInsertedHTML) && serverInsertedHTML.length === 0) {
            return '';
        }
        const stream = await renderToReadableStream(/*#__PURE__*/ _jsxs(_Fragment, {
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
        return streamToString(stream);
    };
}

//# sourceMappingURL=make-get-server-inserted-html.js.map