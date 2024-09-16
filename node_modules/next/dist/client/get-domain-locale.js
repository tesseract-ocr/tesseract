"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getDomainLocale", {
    enumerable: true,
    get: function() {
        return getDomainLocale;
    }
});
const _normalizetrailingslash = require("./normalize-trailing-slash");
const basePath = process.env.__NEXT_ROUTER_BASEPATH || "";
function getDomainLocale(path, locale, locales, domainLocales) {
    if (process.env.__NEXT_I18N_SUPPORT) {
        const normalizeLocalePath = require("./normalize-locale-path").normalizeLocalePath;
        const detectDomainLocale = require("./detect-domain-locale").detectDomainLocale;
        const target = locale || normalizeLocalePath(path, locales).detectedLocale;
        const domain = detectDomainLocale(domainLocales, undefined, target);
        if (domain) {
            const proto = "http" + (domain.http ? "" : "s") + "://";
            const finalLocale = target === domain.defaultLocale ? "" : "/" + target;
            return "" + proto + domain.domain + (0, _normalizetrailingslash.normalizePathTrailingSlash)("" + basePath + finalLocale + path);
        }
        return false;
    } else {
        return false;
    }
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=get-domain-locale.js.map