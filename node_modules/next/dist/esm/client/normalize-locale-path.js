export const normalizeLocalePath = (pathname, locales)=>{
    if (process.env.__NEXT_I18N_SUPPORT) {
        return require('../shared/lib/i18n/normalize-locale-path').normalizeLocalePath(pathname, locales);
    }
    return {
        pathname,
        detectedLocale: undefined
    };
};

//# sourceMappingURL=normalize-locale-path.js.map