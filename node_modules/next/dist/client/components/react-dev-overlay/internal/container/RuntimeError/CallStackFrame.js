"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "CallStackFrame", {
    enumerable: true,
    get: function() {
        return CallStackFrame;
    }
});
const _jsxruntime = require("react/jsx-runtime");
const _stackframe = require("../../helpers/stack-frame");
const _useopenineditor = require("../../helpers/use-open-in-editor");
const _hotlinkedtext = require("../../components/hot-linked-text");
const CallStackFrame = function CallStackFrame(param) {
    let { frame } = param;
    var _frame_originalStackFrame;
    // TODO: ability to expand resolved frames
    // TODO: render error or external indicator
    const f = (_frame_originalStackFrame = frame.originalStackFrame) != null ? _frame_originalStackFrame : frame.sourceStackFrame;
    const hasSource = Boolean(frame.originalCodeFrame);
    const open = (0, _useopenineditor.useOpenInEditor)(hasSource ? {
        file: f.file,
        lineNumber: f.lineNumber,
        column: f.column
    } : undefined);
    // Format method to strip out the webpack layer prefix.
    // e.g. (app-pages-browser)/./app/page.tsx -> ./app/page.tsx
    const formattedMethod = f.methodName.replace(/^\([\w-]+\)\//, '');
    // Formatted file source could be empty. e.g. <anonymous> will be formatted to empty string,
    // we'll skip rendering the frame in this case.
    const fileSource = (0, _stackframe.getFrameSource)(f);
    if (!fileSource) {
        return null;
    }
    return /*#__PURE__*/ (0, _jsxruntime.jsxs)("div", {
        "data-nextjs-call-stack-frame": true,
        children: [
            /*#__PURE__*/ (0, _jsxruntime.jsx)("h3", {
                "data-nextjs-frame-expanded": !frame.ignored,
                children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_hotlinkedtext.HotlinkedText, {
                    text: formattedMethod
                })
            }),
            /*#__PURE__*/ (0, _jsxruntime.jsxs)("div", {
                "data-has-source": hasSource ? 'true' : undefined,
                "data-no-source": hasSource ? undefined : 'true',
                tabIndex: hasSource ? 10 : undefined,
                role: hasSource ? 'link' : undefined,
                onClick: open,
                title: hasSource ? 'Click to open in your editor' : undefined,
                children: [
                    /*#__PURE__*/ (0, _jsxruntime.jsx)("span", {
                        children: fileSource
                    }),
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
            })
        ]
    });
};

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=CallStackFrame.js.map