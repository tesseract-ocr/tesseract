function resolveArray(value) {
    if (Array.isArray(value)) {
        return value;
    }
    return [
        value
    ];
}
function resolveAsArrayOrUndefined(value) {
    if (typeof value === "undefined" || value === null) {
        return undefined;
    }
    return resolveArray(value);
}
export { resolveAsArrayOrUndefined, resolveArray };

//# sourceMappingURL=utils.js.map