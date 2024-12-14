export const detectDomainLocale = function() {
    for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
        args[_key] = arguments[_key];
    }
    if (process.env.__NEXT_I18N_SUPPORT) {
        return require('../shared/lib/i18n/detect-domain-locale').detectDomainLocale(...args);
    }
};

//# sourceMappingURL=detect-domain-locale.js.map