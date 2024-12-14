import { normalizePathTrailingSlash } from './normalize-trailing-slash';
const basePath = process.env.__NEXT_ROUTER_BASEPATH || '';
export function getDomainLocale(path, locale, locales, domainLocales) {
    if (process.env.__NEXT_I18N_SUPPORT) {
        const normalizeLocalePath = require('./normalize-locale-path').normalizeLocalePath;
        const detectDomainLocale = require('./detect-domain-locale').detectDomainLocale;
        const target = locale || normalizeLocalePath(path, locales).detectedLocale;
        const domain = detectDomainLocale(domainLocales, undefined, target);
        if (domain) {
            const proto = "http" + (domain.http ? '' : 's') + "://";
            const finalLocale = target === domain.defaultLocale ? '' : "/" + target;
            return "" + proto + domain.domain + normalizePathTrailingSlash("" + basePath + finalLocale + path);
        }
        return false;
    } else {
        return false;
    }
}

//# sourceMappingURL=get-domain-locale.js.map