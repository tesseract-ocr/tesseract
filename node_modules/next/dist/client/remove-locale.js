"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "removeLocale", {
    enumerable: true,
    get: function() {
        return removeLocale;
    }
});
const _parsepath = require("../shared/lib/router/utils/parse-path");
function removeLocale(path, locale) {
    if (process.env.__NEXT_I18N_SUPPORT) {
        const { pathname } = (0, _parsepath.parsePath)(path);
        const pathLower = pathname.toLowerCase();
        const localeLower = locale == null ? void 0 : locale.toLowerCase();
        return locale && (pathLower.startsWith("/" + localeLower + "/") || pathLower === "/" + localeLower) ? "" + (pathname.length === locale.length + 1 ? "/" : "") + path.slice(locale.length + 1) : path;
    }
    return path;
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=remove-locale.js.map