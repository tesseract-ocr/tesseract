"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    isInternalComponent: null,
    isNonRoutePagesPage: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    isInternalComponent: function() {
        return isInternalComponent;
    },
    isNonRoutePagesPage: function() {
        return isNonRoutePagesPage;
    }
});
function isInternalComponent(pathname) {
    switch(pathname){
        case "next/dist/pages/_app":
        case "next/dist/pages/_document":
            return true;
        default:
            return false;
    }
}
function isNonRoutePagesPage(pathname) {
    return pathname === "/_app" || pathname === "/_document";
}

//# sourceMappingURL=is-internal-component.js.map