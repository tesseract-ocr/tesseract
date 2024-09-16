// This module provides intellisense for all components that has the `"use client"` directive.
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return _default;
    }
});
const _constant = require("../constant");
const _utils = require("../utils");
const errorEntry = {
    getSemanticDiagnostics (source, isClientEntry) {
        const isErrorFile = /[\\/]error\.tsx?$/.test(source.fileName);
        const isGlobalErrorFile = /[\\/]global-error\.tsx?$/.test(source.fileName);
        if (!isErrorFile && !isGlobalErrorFile) return [];
        const ts = (0, _utils.getTs)();
        if (!isClientEntry) {
            // Error components must be Client components
            return [
                {
                    file: source,
                    category: ts.DiagnosticCategory.Error,
                    code: _constant.NEXT_TS_ERRORS.INVALID_ERROR_COMPONENT,
                    messageText: `Error Components must be Client Components, please add the "use client" directive: https://nextjs.org/docs/app/api-reference/file-conventions/error`,
                    start: 0,
                    length: source.text.length
                }
            ];
        }
        return [];
    }
};
const _default = errorEntry;

//# sourceMappingURL=error.js.map