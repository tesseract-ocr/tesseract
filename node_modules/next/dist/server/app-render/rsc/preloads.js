/*

Files in the rsc directory are meant to be packaged as part of the RSC graph using next-app-loader.

*/ "use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    preconnect: null,
    preloadFont: null,
    preloadStyle: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    preconnect: function() {
        return preconnect;
    },
    preloadFont: function() {
        return preloadFont;
    },
    preloadStyle: function() {
        return preloadStyle;
    }
});
const _reactdom = /*#__PURE__*/ _interop_require_default(require("react-dom"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function preloadStyle(href, crossOrigin) {
    const opts = {
        as: "style"
    };
    if (typeof crossOrigin === "string") {
        opts.crossOrigin = crossOrigin;
    }
    _reactdom.default.preload(href, opts);
}
function preloadFont(href, type, crossOrigin) {
    const opts = {
        as: "font",
        type
    };
    if (typeof crossOrigin === "string") {
        opts.crossOrigin = crossOrigin;
    }
    _reactdom.default.preload(href, opts);
}
function preconnect(href, crossOrigin) {
    _reactdom.default.preconnect(href, typeof crossOrigin === "string" ? {
        crossOrigin
    } : undefined);
}

//# sourceMappingURL=preloads.js.map