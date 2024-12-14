export function isClientReference(mod) {
    const defaultExport = (mod == null ? void 0 : mod.default) || mod;
    return (defaultExport == null ? void 0 : defaultExport.$$typeof) === Symbol.for('react.client.reference');
}

//# sourceMappingURL=client-reference.js.map