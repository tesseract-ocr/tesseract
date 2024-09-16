"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "ComponentStackFrameRow", {
    enumerable: true,
    get: function() {
        return ComponentStackFrameRow;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_default._(require("react"));
const _useopenineditor = require("../../helpers/use-open-in-editor");
const _hotlinkedtext = require("../../components/hot-linked-text");
function EditorLink(param) {
    let { children, componentStackFrame: { file, column, lineNumber } } = param;
    const open = (0, _useopenineditor.useOpenInEditor)({
        file,
        column,
        lineNumber
    });
    return /*#__PURE__*/ (0, _jsxruntime.jsxs)("div", {
        tabIndex: 10,
        role: "link",
        onClick: open,
        title: "Click to open in your editor",
        children: [
            children,
            /*#__PURE__*/ (0, _jsxruntime.jsxs)("svg", {
                xmlns: "http://www.w3.org/2000/svg",
                viewBox: "0 0 24 24",
                fill: "none",
                stroke: "currentColor",
                strokeWidth: "2",
                strokeLinecap: "round",
                strokeLinejoin: "round",
                children: [
                    /*#__PURE__*/ (0, _jsxruntime.jsx)("path", {
                        d: "M18 13v6a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h6"
                    }),
                    /*#__PURE__*/ (0, _jsxruntime.jsx)("polyline", {
                        points: "15 3 21 3 21 9"
                    }),
                    /*#__PURE__*/ (0, _jsxruntime.jsx)("line", {
                        x1: "10",
                        y1: "14",
                        x2: "21",
                        y2: "3"
                    })
                ]
            })
        ]
    });
}
function formatLineNumber(lineNumber, column) {
    if (!column) {
        return lineNumber;
    }
    return lineNumber + ":" + column;
}
function LocationLine(param) {
    let { componentStackFrame } = param;
    const { file, lineNumber, column } = componentStackFrame;
    return /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
        children: [
            file,
            " ",
            lineNumber ? "(" + formatLineNumber(lineNumber, column) + ")" : ""
        ]
    });
}
function SourceLocation(param) {
    let { componentStackFrame } = param;
    const { file, canOpenInEditor } = componentStackFrame;
    if (file && canOpenInEditor) {
        return /*#__PURE__*/ (0, _jsxruntime.jsx)(EditorLink, {
            componentStackFrame: componentStackFrame,
            children: /*#__PURE__*/ (0, _jsxruntime.jsx)("span", {
                children: /*#__PURE__*/ (0, _jsxruntime.jsx)(LocationLine, {
                    componentStackFrame: componentStackFrame
                })
            })
        });
    }
    return /*#__PURE__*/ (0, _jsxruntime.jsx)("div", {
        children: /*#__PURE__*/ (0, _jsxruntime.jsx)(LocationLine, {
            componentStackFrame: componentStackFrame
        })
    });
}
function ComponentStackFrameRow(param) {
    let { componentStackFrame } = param;
    const { component } = componentStackFrame;
    return /*#__PURE__*/ (0, _jsxruntime.jsxs)("div", {
        "data-nextjs-component-stack-frame": true,
        children: [
            /*#__PURE__*/ (0, _jsxruntime.jsx)("h3", {
                children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_hotlinkedtext.HotlinkedText, {
                    text: component
                })
            }),
            /*#__PURE__*/ (0, _jsxruntime.jsx)(SourceLocation, {
                componentStackFrame: componentStackFrame
            })
        ]
    });
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=ComponentStackFrameRow.js.map