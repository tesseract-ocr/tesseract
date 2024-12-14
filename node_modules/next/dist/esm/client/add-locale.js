import { normalizePathTrailingSlash } from './normalize-trailing-slash';
export const addLocale = function(path) {
    for(var _len = arguments.length, args = new Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++){
        args[_key - 1] = arguments[_key];
    }
    if (process.env.__NEXT_I18N_SUPPORT) {
        return normalizePathTrailingSlash(require('../shared/lib/router/utils/add-locale').addLocale(path, ...args));
    }
    return path;
};

//# sourceMappingURL=add-locale.js.map