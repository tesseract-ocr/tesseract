"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "useOpenInEditor", {
    enumerable: true,
    get: function() {
        return useOpenInEditor;
    }
});
const _react = require("react");
function useOpenInEditor(param) {
    let { file, lineNumber, column } = param === void 0 ? {} : param;
    const openInEditor = (0, _react.useCallback)(()=>{
        if (file == null || lineNumber == null || column == null) return;
        const params = new URLSearchParams();
        params.append("file", file);
        params.append("lineNumber", String(lineNumber));
        params.append("column", String(column));
        self.fetch((process.env.__NEXT_ROUTER_BASEPATH || "") + "/__nextjs_launch-editor?" + params.toString()).then(()=>{}, ()=>{
            console.error("There was an issue opening this code in your editor.");
        });
    }, [
        file,
        lineNumber,
        column
    ]);
    return openInEditor;
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=use-open-in-editor.js.map