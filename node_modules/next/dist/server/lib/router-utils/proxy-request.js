"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "proxyRequest", {
    enumerable: true,
    get: function() {
        return proxyRequest;
    }
});
const _url = /*#__PURE__*/ _interop_require_default(require("url"));
const _serverrouteutils = require("../../server-route-utils");
const _stream = require("stream");
const _detachedpromise = require("../../../lib/detached-promise");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function proxyRequest(req, res, parsedUrl, upgradeHead, reqBody, proxyTimeout) {
    const { query } = parsedUrl;
    delete parsedUrl.query;
    parsedUrl.search = (0, _serverrouteutils.stringifyQuery)(req, query);
    const target = _url.default.format(parsedUrl);
    const HttpProxy = require('next/dist/compiled/http-proxy');
    const proxy = new HttpProxy({
        target,
        changeOrigin: true,
        ignorePath: true,
        ws: true,
        // we limit proxy requests to 30s by default, in development
        // we don't time out WebSocket requests to allow proxying
        proxyTimeout: proxyTimeout === null ? undefined : proxyTimeout || 30000,
        headers: {
            'x-forwarded-host': req.headers.host || ''
        }
    });
    let finished = false;
    // http-proxy does not properly detect a client disconnect in newer
    // versions of Node.js. This is caused because it only listens for the
    // `aborted` event on the our request object, but it also fully reads
    // and closes the request object. Node **will not** fire `aborted` when
    // the request is already closed. Listening for `close` on our response
    // object will detect the disconnect, and we can abort the proxy's
    // connection.
    proxy.on('proxyReq', (proxyReq)=>{
        res.on('close', ()=>proxyReq.destroy());
    });
    proxy.on('proxyRes', (proxyRes)=>{
        if (res.destroyed) {
            proxyRes.destroy();
        } else {
            res.on('close', ()=>proxyRes.destroy());
        }
    });
    proxy.on('proxyRes', (proxyRes, innerReq, innerRes)=>{
        const cleanup = (err)=>{
            // cleanup event listeners to allow clean garbage collection
            proxyRes.removeListener('error', cleanup);
            proxyRes.removeListener('close', cleanup);
            innerRes.removeListener('error', cleanup);
            innerRes.removeListener('close', cleanup);
            // destroy all source streams to propagate the caught event backward
            innerReq.destroy(err);
            proxyRes.destroy(err);
        };
        proxyRes.once('error', cleanup);
        proxyRes.once('close', cleanup);
        innerRes.once('error', cleanup);
        innerRes.once('close', cleanup);
    });
    const detached = new _detachedpromise.DetachedPromise();
    proxy.on('error', (err)=>{
        console.error(`Failed to proxy ${target}`, err);
        if (!finished) {
            finished = true;
            detached.reject(err);
            if (!res.destroyed) {
                if (!(res instanceof _stream.Duplex)) {
                    res.statusCode = 500;
                }
                res.end('Internal Server Error');
            }
        }
    });
    // If upgrade head is present or the response is a Duplex stream, treat as
    // WebSocket request.
    if (upgradeHead || res instanceof _stream.Duplex) {
        proxy.on('proxyReqWs', (proxyReq)=>{
            proxyReq.on('close', ()=>{
                if (!finished) {
                    finished = true;
                    detached.resolve(true);
                }
            });
        });
        proxy.ws(req, res, upgradeHead);
        detached.resolve(true);
    } else {
        proxy.on('proxyReq', (proxyReq)=>{
            proxyReq.on('close', ()=>{
                if (!finished) {
                    finished = true;
                    detached.resolve(true);
                }
            });
        });
        proxy.web(req, res, {
            buffer: reqBody
        });
    }
    // When the proxy finishes proxying the request, shut down the proxy.
    return detached.promise.finally(()=>{
        proxy.close();
    });
}

//# sourceMappingURL=proxy-request.js.map