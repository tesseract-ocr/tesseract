const symbolError = Symbol.for('NextjsError');
export function getErrorSource(error) {
    return error[symbolError] || null;
}
export function decorateServerError(error, type) {
    Object.defineProperty(error, symbolError, {
        writable: false,
        enumerable: false,
        configurable: false,
        value: type
    });
}

//# sourceMappingURL=error-source.js.map