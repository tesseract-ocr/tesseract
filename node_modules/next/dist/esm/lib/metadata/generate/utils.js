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
export { resolveAsArrayOrUndefined, resolveArray, getOrigin };

//# sourceMappingURL=utils.js.map