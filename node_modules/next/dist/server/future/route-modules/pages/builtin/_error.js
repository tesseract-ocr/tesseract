"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "routeModule", {
    enumerable: true,
    get: function() {
        return routeModule;
    }
});
const _document = /*#__PURE__*/ _interop_require_default(require("../../../../../pages/_document"));
const _app = /*#__PURE__*/ _interop_require_default(require("../../../../../pages/_app"));
const _routekind = require("../../../route-kind");
const _error = /*#__PURE__*/ _interop_require_wildcard(require("../../../../../pages/_error"));
const _module = /*#__PURE__*/ _interop_require_default(require("../module"));
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
const routeModule = new _module.default({
    // TODO: add descriptor for internal error page
    definition: {
        kind: _routekind.RouteKind.PAGES,
        page: "/_error",
        pathname: "/_error",
        filename: "",
        bundlePath: ""
    },
    components: {
        App: _app.default,
        Document: _document.default
    },
    userland: _error
});

//# sourceMappingURL=_error.js.map