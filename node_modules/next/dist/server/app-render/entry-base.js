// eslint-disable-next-line import/no-extraneous-dependencies
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    ClientPageRoot: null,
    ClientSegmentRoot: null,
    HTTPAccessFallbackBoundary: null,
    LayoutRouter: null,
    MetadataBoundary: null,
    OutletBoundary: null,
    Postpone: null,
    RenderFromTemplateContext: null,
    ViewportBoundary: null,
    actionAsyncStorage: null,
    collectSegmentData: null,
    createMetadataComponents: null,
    createPrerenderParamsForClientSegment: null,
    createPrerenderSearchParamsForClientPage: null,
    createServerParamsForMetadata: null,
    createServerParamsForServerSegment: null,
    createServerSearchParamsForMetadata: null,
    createServerSearchParamsForServerPage: null,
    createTemporaryReferenceSet: null,
    decodeAction: null,
    decodeFormState: null,
    decodeReply: null,
    patchFetch: null,
    preconnect: null,
    preloadFont: null,
    preloadStyle: null,
    prerender: null,
    renderToReadableStream: null,
    serverHooks: null,
    taintObjectReference: null,
    workAsyncStorage: null,
    workUnitAsyncStorage: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    ClientPageRoot: function() {
        return _clientpage.ClientPageRoot;
    },
    ClientSegmentRoot: function() {
        return _clientsegment.ClientSegmentRoot;
    },
    HTTPAccessFallbackBoundary: function() {
        return _errorboundary.HTTPAccessFallbackBoundary;
    },
    LayoutRouter: function() {
        return _layoutrouter.default;
    },
    MetadataBoundary: function() {
        return _metadataboundary.MetadataBoundary;
    },
    OutletBoundary: function() {
        return _metadataboundary.OutletBoundary;
    },
    Postpone: function() {
        return _postpone.Postpone;
    },
    RenderFromTemplateContext: function() {
        return _renderfromtemplatecontext.default;
    },
    ViewportBoundary: function() {
        return _metadataboundary.ViewportBoundary;
    },
    actionAsyncStorage: function() {
        return _actionasyncstorageexternal.actionAsyncStorage;
    },
    collectSegmentData: function() {
        return _collectsegmentdata.collectSegmentData;
    },
    createMetadataComponents: function() {
        return _metadata.createMetadataComponents;
    },
    createPrerenderParamsForClientSegment: function() {
        return _params.createPrerenderParamsForClientSegment;
    },
    createPrerenderSearchParamsForClientPage: function() {
        return _searchparams.createPrerenderSearchParamsForClientPage;
    },
    createServerParamsForMetadata: function() {
        return _params.createServerParamsForMetadata;
    },
    createServerParamsForServerSegment: function() {
        return _params.createServerParamsForServerSegment;
    },
    createServerSearchParamsForMetadata: function() {
        return _searchparams.createServerSearchParamsForMetadata;
    },
    createServerSearchParamsForServerPage: function() {
        return _searchparams.createServerSearchParamsForServerPage;
    },
    createTemporaryReferenceSet: function() {
        return _serveredge.createTemporaryReferenceSet;
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
    prerender: function() {
        return _staticedge.prerender;
    },
    renderToReadableStream: function() {
        return _serveredge.renderToReadableStream;
    },
    serverHooks: function() {
        return _hooksservercontext;
    },
    taintObjectReference: function() {
        return _taint.taintObjectReference;
    },
    workAsyncStorage: function() {
        return _workasyncstorageexternal.workAsyncStorage;
    },
    workUnitAsyncStorage: function() {
        return _workunitasyncstorageexternal.workUnitAsyncStorage;
    }
});
const _serveredge = require("react-server-dom-webpack/server.edge");
const _staticedge = require("react-server-dom-webpack/static.edge");
const _layoutrouter = /*#__PURE__*/ _interop_require_default(require("../../client/components/layout-router"));
const _renderfromtemplatecontext = /*#__PURE__*/ _interop_require_default(require("../../client/components/render-from-template-context"));
const _workasyncstorageexternal = require("../app-render/work-async-storage.external");
const _workunitasyncstorageexternal = require("./work-unit-async-storage.external");
const _actionasyncstorageexternal = require("../app-render/action-async-storage.external");
const _clientpage = require("../../client/components/client-page");
const _clientsegment = require("../../client/components/client-segment");
const _searchparams = require("../request/search-params");
const _params = require("../request/params");
const _hooksservercontext = /*#__PURE__*/ _interop_require_wildcard(require("../../client/components/hooks-server-context"));
const _errorboundary = require("../../client/components/http-access-fallback/error-boundary");
const _metadata = require("../../lib/metadata/metadata");
const _patchfetch = require("../lib/patch-fetch");
require("../../client/components/error-boundary");
const _metadataboundary = require("../../lib/metadata/metadata-boundary");
const _preloads = require("./rsc/preloads");
const _postpone = require("./rsc/postpone");
const _taint = require("./rsc/taint");
const _collectsegmentdata = require("./collect-segment-data");
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
        workAsyncStorage: _workasyncstorageexternal.workAsyncStorage,
        workUnitAsyncStorage: _workunitasyncstorageexternal.workUnitAsyncStorage
    });
}

//# sourceMappingURL=entry-base.js.map