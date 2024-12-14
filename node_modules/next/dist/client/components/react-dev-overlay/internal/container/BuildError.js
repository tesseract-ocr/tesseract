"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    BuildError: null,
    styles: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    BuildError: function() {
        return BuildError;
    },
    styles: function() {
        return styles;
    }
});
const _interop_require_wildcard = require("@swc/helpers/_/_interop_require_wildcard");
const _tagged_template_literal_loose = require("@swc/helpers/_/_tagged_template_literal_loose");
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_wildcard._(require("react"));
const _Dialog = require("../components/Dialog");
const _Overlay = require("../components/Overlay");
const _Terminal = require("../components/Terminal");
const _VersionStalenessInfo = require("../components/VersionStalenessInfo");
const _nooptemplate = require("../helpers/noop-template");
function _templateObject() {
    const data = _tagged_template_literal_loose._([
        "\n  h1.nextjs__container_errors_label {\n    font-size: var(--size-font-big);\n    line-height: var(--size-font-bigger);\n    font-weight: bold;\n    margin: var(--size-gap-double) 0;\n  }\n  .nextjs-container-errors-header p {\n    font-size: var(--size-font-small);\n    line-height: var(--size-font-big);\n    white-space: pre-wrap;\n  }\n  .nextjs-container-errors-body footer {\n    margin-top: var(--size-gap);\n  }\n  .nextjs-container-errors-body footer p {\n    margin: 0;\n  }\n\n  .nextjs-container-errors-body small {\n    color: var(--color-font);\n  }\n"
    ]);
    _templateObject = function() {
        return data;
    };
    return data;
}
const BuildError = function BuildError(param) {
    let { message, versionInfo } = param;
    const noop = _react.useCallback(()=>{}, []);
    return /*#__PURE__*/ (0, _jsxruntime.jsx)(_Overlay.Overlay, {
        fixed: true,
        children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_Dialog.Dialog, {
            type: "error",
            "aria-labelledby": "nextjs__container_error_label",
            "aria-describedby": "nextjs__container_error_desc",
            onClose: noop,
            children: /*#__PURE__*/ (0, _jsxruntime.jsxs)(_Dialog.DialogContent, {
                children: [
                    /*#__PURE__*/ (0, _jsxruntime.jsxs)(_Dialog.DialogHeader, {
                        className: "nextjs-container-errors-header",
                        children: [
                            /*#__PURE__*/ (0, _jsxruntime.jsx)("h1", {
                                id: "nextjs__container_errors_label",
                                className: "nextjs__container_errors_label",
                                children: 'Build Error'
                            }),
                            /*#__PURE__*/ (0, _jsxruntime.jsx)(_VersionStalenessInfo.VersionStalenessInfo, {
                                versionInfo: versionInfo
                            }),
                            /*#__PURE__*/ (0, _jsxruntime.jsx)("p", {
                                id: "nextjs__container_errors_desc",
                                className: "nextjs__container_errors_desc",
                                children: "Failed to compile"
                            })
                        ]
                    }),
                    /*#__PURE__*/ (0, _jsxruntime.jsxs)(_Dialog.DialogBody, {
                        className: "nextjs-container-errors-body",
                        children: [
                            /*#__PURE__*/ (0, _jsxruntime.jsx)(_Terminal.Terminal, {
                                content: message
                            }),
                            /*#__PURE__*/ (0, _jsxruntime.jsx)("footer", {
                                children: /*#__PURE__*/ (0, _jsxruntime.jsx)("p", {
                                    id: "nextjs__container_build_error_desc",
                                    children: /*#__PURE__*/ (0, _jsxruntime.jsx)("small", {
                                        children: "This error occurred during the build process and can only be dismissed by fixing the error."
                                    })
                                })
                            })
                        ]
                    })
                ]
            })
        })
    });
};
const styles = (0, _nooptemplate.noop)(_templateObject());

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=BuildError.js.map