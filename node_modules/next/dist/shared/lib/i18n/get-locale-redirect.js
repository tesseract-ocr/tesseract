"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getLocaleRedirect", {
    enumerable: true,
    get: function() {
        return getLocaleRedirect;
    }
});
const _acceptheader = require("../../../server/accept-header");
const _denormalizepagepath = require("../page-path/denormalize-page-path");
const _detectdomainlocale = require("./detect-domain-locale");
const _formaturl = require("../router/utils/format-url");
const _getcookieparser = require("../../../server/api-utils/get-cookie-parser");
function getLocaleFromCookie(i18n, headers) {
    if (headers === void 0) headers = {};
    var _getCookieParser_NEXT_LOCALE, _getCookieParser;
    const nextLocale = (_getCookieParser = (0, _getcookieparser.getCookieParser)(headers || {})()) == null ? void 0 : (_getCookieParser_NEXT_LOCALE = _getCookieParser.NEXT_LOCALE) == null ? void 0 : _getCookieParser_NEXT_LOCALE.toLowerCase();
    return nextLocale ? i18n.locales.find((locale)=>nextLocale === locale.toLowerCase()) : undefined;
}
function detectLocale(param) {
    let { i18n, headers, domainLocale, preferredLocale, pathLocale } = param;
    return pathLocale || (domainLocale == null ? void 0 : domainLocale.defaultLocale) || getLocaleFromCookie(i18n, headers) || preferredLocale || i18n.defaultLocale;
}
function getAcceptPreferredLocale(i18n, headers) {
    if ((headers == null ? void 0 : headers['accept-language']) && !Array.isArray(headers['accept-language'])) {
        try {
            return (0, _acceptheader.acceptLanguage)(headers['accept-language'], i18n.locales);
        } catch (err) {}
    }
}
function getLocaleRedirect(param) {
    let { defaultLocale, domainLocale, pathLocale, headers, nextConfig, urlParsed } = param;
    if (nextConfig.i18n && nextConfig.i18n.localeDetection !== false && (0, _denormalizepagepath.denormalizePagePath)(urlParsed.pathname) === '/') {
        const preferredLocale = getAcceptPreferredLocale(nextConfig.i18n, headers);
        const detectedLocale = detectLocale({
            i18n: nextConfig.i18n,
            preferredLocale,
            headers,
            pathLocale,
            domainLocale
        });
        const preferredDomain = (0, _detectdomainlocale.detectDomainLocale)(nextConfig.i18n.domains, undefined, preferredLocale);
        if (domainLocale && preferredDomain) {
            const isPDomain = preferredDomain.domain === domainLocale.domain;
            const isPLocale = preferredDomain.defaultLocale === preferredLocale;
            if (!isPDomain || !isPLocale) {
                const scheme = "http" + (preferredDomain.http ? '' : 's');
                const rlocale = isPLocale ? '' : preferredLocale;
                return scheme + "://" + preferredDomain.domain + "/" + rlocale;
            }
        }
        if (detectedLocale.toLowerCase() !== defaultLocale.toLowerCase()) {
            return (0, _formaturl.formatUrl)({
                ...urlParsed,
                pathname: (nextConfig.basePath || '') + "/" + detectedLocale + (nextConfig.trailingSlash ? '/' : '')
            });
        }
    }
}

//# sourceMappingURL=get-locale-redirect.js.map