"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return nextInvalidImportErrorLoader;
    }
});
function nextInvalidImportErrorLoader() {
    const { message } = this.getOptions();
    throw new Error(message);
}

//# sourceMappingURL=next-invalid-import-error-loader.js.map