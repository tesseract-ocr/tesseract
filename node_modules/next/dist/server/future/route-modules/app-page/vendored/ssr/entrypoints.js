"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    React: null,
    ReactDOM: null,
    ReactDOMServerEdge: null,
    ReactJsxDevRuntime: null,
    ReactJsxRuntime: null,
    ReactServerDOMTurbopackClientEdge: null,
    ReactServerDOMWebpackClientEdge: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    React: function() {
        return _react;
    },
    ReactDOM: function() {
        return _serverrenderingstub;
    },
    ReactDOMServerEdge: function() {
        return _serveredge;
    },
    ReactJsxDevRuntime: function() {
        return _jsxdevruntime;
    },
    ReactJsxRuntime: function() {
        return _jsxruntime;
    },
    ReactServerDOMTurbopackClientEdge: function() {
        return ReactServerDOMTurbopackClientEdge;
    },
    ReactServerDOMWebpackClientEdge: function() {
        return ReactServerDOMWebpackClientEdge;
    }
});
const _react = /*#__PURE__*/ _interop_require_wildcard(require("react"));
const _serverrenderingstub = /*#__PURE__*/ _interop_require_wildcard(require("react-dom/server-rendering-stub"));
const _jsxdevruntime = /*#__PURE__*/ _interop_require_wildcard(require("react/jsx-dev-runtime"));
const _jsxruntime = /*#__PURE__*/ _interop_require_wildcard(require("react/jsx-runtime"));
const _serveredge = /*#__PURE__*/ _interop_require_wildcard(require("react-dom/server.edge"));
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
function getAltProxyForBindingsDEV(type, pkg) {
    if (process.env.NODE_ENV === "development") {
        const altType = type === "Turbopack" ? "Webpack" : "Turbopack";
        const altPkg = pkg.replace(new RegExp(type, "gi"), altType.toLowerCase());
        return new Proxy({}, {
            get (_, prop) {
                throw new Error(`Expected to use ${type} bindings (${pkg}) for React but the current process is referencing '${prop}' from the ${altType} bindings (${altPkg}). This is likely a bug in our integration of the Next.js server runtime.`);
            }
        });
    }
}
let ReactServerDOMTurbopackClientEdge, ReactServerDOMWebpackClientEdge;
if (process.env.TURBOPACK) {
    // eslint-disable-next-line import/no-extraneous-dependencies
    ReactServerDOMTurbopackClientEdge = require("react-server-dom-turbopack/client.edge");
    if (process.env.NODE_ENV === "development") {
        ReactServerDOMWebpackClientEdge = getAltProxyForBindingsDEV("Turbopack", "react-server-dom-turbopack/client.edge");
    }
} else {
    // eslint-disable-next-line import/no-extraneous-dependencies
    ReactServerDOMWebpackClientEdge = require("react-server-dom-webpack/client.edge");
    if (process.env.NODE_ENV === "development") {
        ReactServerDOMTurbopackClientEdge = getAltProxyForBindingsDEV("Webpack", "react-server-dom-webpack/client.edge");
    }
}

//# sourceMappingURL=entrypoints.js.map