"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getOrigin: null,
    resolveArray: null,
    resolveAsArrayOrUndefined: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getOrigin: function() {
        return getOrigin;
    },
    resolveArray: function() {
        return resolveArray;
    },
    resolveAsArrayOrUndefined: function() {
        return resolveAsArrayOrUndefined;
    }
});
function resolveArray(value) {
    if (Array.isArray(value)) {
        return value;
    }
    return [
        value
    ];
}
function resolveAsArrayOrUndefined(value) {
    if (typeof value === 'undefined' || value === null) {
        return undefined;
    }
    return resolveArray(value);
}
function getOrigin(url) {
    let origin = undefined;
    if (typeof url === 'string') {
        try {
            url = new URL(url);
            origin = url.origin;
        } catch  {}
    }
    return origin;
}

//# sourceMappingURL=utils.js.map