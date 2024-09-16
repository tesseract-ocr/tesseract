"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "RootLayoutMissingTagsError", {
    enumerable: true,
    get: function() {
        return RootLayoutMissingTagsError;
    }
});
const _interop_require_wildcard = require("@swc/helpers/_/_interop_require_wildcard");
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_wildcard._(require("react"));
const _Dialog = require("../components/Dialog");
const _Overlay = require("../components/Overlay");
const _VersionStalenessInfo = require("../components/VersionStalenessInfo");
const _hotlinkedtext = require("../components/hot-linked-text");
const RootLayoutMissingTagsError = function RootLayoutMissingTagsError(param) {
    let { missingTags, versionInfo } = param;
    const noop = _react.useCallback(()=>{}, []);
    return /*#__PURE__*/ (0, _jsxruntime.jsx)(_Overlay.Overlay, {
        children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_Dialog.Dialog, {
            type: "error",
            "aria-labelledby": "nextjs__container_errors_label",
            "aria-describedby": "nextjs__container_errors_desc",
            onClose: noop,
            children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_Dialog.DialogContent, {
                children: /*#__PURE__*/ (0, _jsxruntime.jsxs)(_Dialog.DialogHeader, {
                    className: "nextjs-container-errors-header",
                    children: [
                        /*#__PURE__*/ (0, _jsxruntime.jsx)("h3", {
                            id: "nextjs__container_errors_label",
                            children: "Missing required html tags"
                        }),
                        versionInfo ? /*#__PURE__*/ (0, _jsxruntime.jsx)(_VersionStalenessInfo.VersionStalenessInfo, {
                            ...versionInfo
                        }) : null,
                        /*#__PURE__*/ (0, _jsxruntime.jsx)("p", {
                            id: "nextjs__container_errors_desc",
                            className: "nextjs__container_errors_desc",
                            children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_hotlinkedtext.HotlinkedText, {
                                text: "The following tags are missing in the Root Layout: " + missingTags.map((tagName)=>"<" + tagName + ">").join(", ") + ".\nRead more at https://nextjs.org/docs/messages/missing-root-layout-tags"
                            })
                        })
                    ]
                })
            })
        })
    });
};

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=root-layout-missing-tags-error.js.map