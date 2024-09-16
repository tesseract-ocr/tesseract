"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "ImageConfigContext", {
    enumerable: true,
    get: function() {
        return ImageConfigContext;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _react = /*#__PURE__*/ _interop_require_default._(require("react"));
const _imageconfig = require("./image-config");
const ImageConfigContext = _react.default.createContext(_imageconfig.imageConfigDefault);
if (process.env.NODE_ENV !== "production") {
    ImageConfigContext.displayName = "ImageConfigContext";
}

//# sourceMappingURL=image-config-context.shared-runtime.js.map