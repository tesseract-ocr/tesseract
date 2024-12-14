"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    interceptTestApis: null,
    wrapRequestHandlerNode: null,
    wrapRequestHandlerWorker: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    interceptTestApis: function() {
        return interceptTestApis;
    },
    wrapRequestHandlerNode: function() {
        return wrapRequestHandlerNode;
    },
    wrapRequestHandlerWorker: function() {
        return wrapRequestHandlerWorker;
    }
});
const _context = require("./context");
const _fetch = require("./fetch");
const _httpget = require("./httpget");
const reader = {
    url (req) {
        return req.url ?? '';
    },
    header (req, name) {
        const h = req.headers[name];
        if (h === undefined || h === null) {
            return null;
        }
        if (typeof h === 'string') {
            return h;
        }
        return h[0] ?? null;
    }
};
function interceptTestApis() {
    const originalFetch = global.fetch;
    const restoreFetch = (0, _fetch.interceptFetch)(originalFetch);
    const restoreHttpGet = (0, _httpget.interceptHttpGet)(originalFetch);
    // Cleanup.
    return ()=>{
        restoreFetch();
        restoreHttpGet();
    };
}
function wrapRequestHandlerWorker(handler) {
    return (req, res)=>(0, _context.withRequest)(req, reader, ()=>handler(req, res));
}
function wrapRequestHandlerNode(handler) {
    return (req, res, parsedUrl)=>(0, _context.withRequest)(req, reader, ()=>handler(req, res, parsedUrl));
}

//# sourceMappingURL=server.js.map