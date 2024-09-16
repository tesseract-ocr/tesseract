// eslint-disable-next-line import/no-extraneous-dependencies
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    AppRouter: null,
    ClientPageRoot: null,
    LayoutRouter: null,
    NotFoundBoundary: null,
    Postpone: null,
    RenderFromTemplateContext: null,
    actionAsyncStorage: null,
    createDynamicallyTrackedSearchParams: null,
    createUntrackedSearchParams: null,
    decodeAction: null,
    decodeFormState: null,
    decodeReply: null,
    patchFetch: null,
    preconnect: null,
    preloadFont: null,
    preloadStyle: null,
    renderToReadableStream: null,
    requestAsyncStorage: null,
    serverHooks: null,
    staticGenerationAsyncStorage: null,
    taintObjectReference: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    AppRouter: function() {
        return _approuter.default;
    },
    ClientPageRoot: function() {
        return _clientpage.ClientPageRoot;
    },
    LayoutRouter: function() {
        return _layoutrouter.default;
    },
    NotFoundBoundary: function() {
        return _notfoundboundary.NotFoundBoundary;
    },
    Postpone: function() {
        return _postpone.Postpone;
    },
    RenderFromTemplateContext: function() {
        return _renderfromtemplatecontext.default;
    },
    actionAsyncStorage: function() {
        return _actionasyncstorageexternal.actionAsyncStorage;
    },
    createDynamicallyTrackedSearchParams: function() {
        return _searchparams.createDynamicallyTrackedSearchParams;
    },
    createUntrackedSearchParams: function() {
        return _searchparams.createUntrackedSearchParams;
    },
    decodeAction: function() {
        return _serveredge.decodeAction;
    },
    decodeFormState: function() {
        return _serveredge.decodeFormState;
    },
    decodeReply: function() {
        return _serveredge.decodeReply;
    },
    patchFetch: function() {
        return patchFetch;
    },
    preconnect: function() {
        return _preloads.preconnect;
    },
    preloadFont: function() {
        return _preloads.preloadFont;
    },
    preloadStyle: function() {
        return _preloads.preloadStyle;
    },
    renderToReadableStream: function() {
        return _serveredge.renderToReadableStream;
    },
    requestAsyncStorage: function() {
        return _requestasyncstorageexternal.requestAsyncStorage;
    },
    serverHooks: function() {
        return _hooksservercontext;
    },
    staticGenerationAsyncStorage: function() {
        return _staticgenerationasyncstorageexternal.staticGenerationAsyncStorage;
    },
    taintObjectReference: function() {
        return _taint.taintObjectReference;
    }
});
const _serveredge = require("react-server-dom-webpack/server.edge");
const _approuter = /*#__PURE__*/ _interop_require_default(require("../../client/components/app-router"));
const _layoutrouter = /*#__PURE__*/ _interop_require_default(require("../../client/components/layout-router"));
const _renderfromtemplatecontext = /*#__PURE__*/ _interop_require_default(require("../../client/components/render-from-template-context"));
const _staticgenerationasyncstorageexternal = require("../../client/components/static-generation-async-storage.external");
const _requestasyncstorageexternal = require("../../client/components/request-async-storage.external");
const _actionasyncstorageexternal = require("../../client/components/action-async-storage.external");
const _clientpage = require("../../client/components/client-page");
const _searchparams = require("../../client/components/search-params");
const _hooksservercontext = /*#__PURE__*/ _interop_require_wildcard(require("../../client/components/hooks-server-context"));
const _notfoundboundary = require("../../client/components/not-found-boundary");
const _patchfetch = require("../lib/patch-fetch");
require("../../client/components/error-boundary");
const _preloads = require("../../server/app-render/rsc/preloads");
const _postpone = require("../../server/app-render/rsc/postpone");
const _taint = require("../../server/app-render/rsc/taint");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function _getRequireWildcardCache(nodeInterop) {
    if (typeof WeakMap !== "function") return null;
    var cacheBabelInterop = new WeakMap();
    var cacheNodeInterop = new WeakMap();
    return (_getRequireWildcardCache = function(nodeInterop) {
        return nodeInterop ? cacheNodeInterop : cacheBabelInterop;
    })(nodeInterop);
}
function _interop_require_wildcard(obj, nodeInterop) {
    if (!nodeInterop && obj && obj.__esModule) {
        return obj;
    }
    if (obj === null || typeof obj !== "object" && typeof obj !== "function") {
        return {
            default: obj
        };
    }
    var cache = _getRequireWildcardCache(nodeInterop);
    if (cache && cache.has(obj)) {
        return cache.get(obj);
    }
    var newObj = {
        __proto__: null
    };
    var hasPropertyDescriptor = Object.defineProperty && Object.getOwnPropertyDescriptor;
    for(var key in obj){
        if (key !== "default" && Object.prototype.hasOwnProperty.call(obj, key)) {
            var desc = hasPropertyDescriptor ? Object.getOwnPropertyDescriptor(obj, key) : null;
            if (desc && (desc.get || desc.set)) {
                Object.defineProperty(newObj, key, desc);
            } else {
                newObj[key] = obj[key];
            }
        }
    }
    newObj.default = obj;
    if (cache) {
        cache.set(obj, newObj);
    }
    return newObj;
}
// patchFetch makes use of APIs such as `React.unstable_postpone` which are only available
// in the experimental channel of React, so export it from here so that it comes from the bundled runtime
function patchFetch() {
    return (0, _patchfetch.patchFetch)({
        serverHooks: _hooksservercontext,
        staticGenerationAsyncStorage: _staticgenerationasyncstorageexternal.staticGenerationAsyncStorage
    });
}

//# sourceMappingURL=entry-base.js.map