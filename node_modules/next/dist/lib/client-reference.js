"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "isClientReference", {
    enumerable: true,
    get: function() {
        return isClientReference;
    }
});
function isClientReference(mod) {
    const defaultExport = (mod == null ? void 0 : mod.default) || mod;
    return (defaultExport == null ? void 0 : defaultExport.$$typeof) === Symbol.for("react.client.reference");
}

//# sourceMappingURL=client-reference.js.map