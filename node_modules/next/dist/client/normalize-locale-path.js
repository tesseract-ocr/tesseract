"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "normalizeLocalePath", {
    enumerable: true,
    get: function() {
        return normalizeLocalePath;
    }
});
const normalizeLocalePath = (pathname, locales)=>{
    if (process.env.__NEXT_I18N_SUPPORT) {
        return require('../shared/lib/i18n/normalize-locale-path').normalizeLocalePath(pathname, locales);
    }
    return {
        pathname,
        detectedLocale: undefined
    };
};

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=normalize-locale-path.js.map