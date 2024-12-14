"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    dynamicParamTypes: null,
    getShortDynamicParamType: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    dynamicParamTypes: function() {
        return dynamicParamTypes;
    },
    getShortDynamicParamType: function() {
        return getShortDynamicParamType;
    }
});
const dynamicParamTypes = {
    catchall: 'c',
    'catchall-intercepted': 'ci',
    'optional-catchall': 'oc',
    dynamic: 'd',
    'dynamic-intercepted': 'di'
};
function getShortDynamicParamType(type) {
    const short = dynamicParamTypes[type];
    if (!short) {
        throw new Error('Unknown dynamic param type');
    }
    return short;
}

//# sourceMappingURL=get-short-dynamic-param-type.js.map