"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    RuntimeError: null,
    styles: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    RuntimeError: function() {
        return RuntimeError;
    },
    styles: function() {
        return styles;
    }
});
const _interop_require_wildcard = require("@swc/helpers/_/_interop_require_wildcard");
const _tagged_template_literal_loose = require("@swc/helpers/_/_tagged_template_literal_loose");
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_wildcard._(require("react"));
const _CodeFrame = require("../../components/CodeFrame");
const _nooptemplate = require("../../helpers/noop-template");
const _groupstackframesbyframework = require("../../helpers/group-stack-frames-by-framework");
const _GroupedStackFrames = require("./GroupedStackFrames");
function _templateObject() {
    const data = _tagged_template_literal_loose._([
        "\n  button[data-nextjs-data-runtime-error-collapsed-action] {\n    background: none;\n    border: none;\n    padding: 0;\n    font-size: var(--size-font-small);\n    line-height: var(--size-font-bigger);\n    color: var(--color-accents-3);\n  }\n\n  [data-nextjs-call-stack-frame]:not(:last-child),\n  [data-nextjs-component-stack-frame]:not(:last-child) {\n    margin-bottom: var(--size-gap-double);\n  }\n\n  [data-nextjs-call-stack-frame] > h3,\n  [data-nextjs-component-stack-frame] > h3 {\n    margin-top: 0;\n    margin-bottom: var(--size-gap);\n    font-family: var(--font-stack-monospace);\n    font-size: var(--size-font);\n    color: #222;\n  }\n  [data-nextjs-call-stack-frame] > h3[data-nextjs-frame-expanded='false'] {\n    color: #666;\n  }\n  [data-nextjs-call-stack-frame] > div,\n  [data-nextjs-component-stack-frame] > div {\n    display: flex;\n    align-items: center;\n    padding-left: calc(var(--size-gap) + var(--size-gap-half));\n    font-size: var(--size-font-small);\n    color: #999;\n  }\n  [data-nextjs-call-stack-frame] > div > svg,\n  [data-nextjs-component-stack-frame] > [role='link'] > svg {\n    width: auto;\n    height: var(--size-font-small);\n    margin-left: var(--size-gap);\n    flex-shrink: 0;\n\n    display: none;\n  }\n\n  [data-nextjs-call-stack-frame] > div[data-has-source],\n  [data-nextjs-component-stack-frame] > [role='link'] {\n    cursor: pointer;\n  }\n  [data-nextjs-call-stack-frame] > div[data-has-source]:hover,\n  [data-nextjs-component-stack-frame] > [role='link']:hover {\n    text-decoration: underline dotted;\n  }\n  [data-nextjs-call-stack-frame] > div[data-has-source] > svg,\n  [data-nextjs-component-stack-frame] > [role='link'] > svg {\n    display: unset;\n  }\n\n  [data-nextjs-call-stack-framework-icon] {\n    margin-right: var(--size-gap);\n  }\n  [data-nextjs-call-stack-framework-icon='next'] > mask {\n    mask-type: alpha;\n  }\n  [data-nextjs-call-stack-framework-icon='react'] {\n    color: rgb(20, 158, 202);\n  }\n  [data-nextjs-collapsed-call-stack-details][open]\n    [data-nextjs-call-stack-chevron-icon] {\n    transform: rotate(90deg);\n  }\n  [data-nextjs-collapsed-call-stack-details] summary {\n    display: flex;\n    align-items: center;\n    margin-bottom: var(--size-gap);\n    list-style: none;\n  }\n  [data-nextjs-collapsed-call-stack-details] summary::-webkit-details-marker {\n    display: none;\n  }\n\n  [data-nextjs-collapsed-call-stack-details] h3 {\n    color: #666;\n  }\n  [data-nextjs-collapsed-call-stack-details] [data-nextjs-call-stack-frame] {\n    margin-bottom: var(--size-gap-double);\n  }\n\n  [data-nextjs-container-errors-pseudo-html] {\n    position: relative;\n  }\n  [data-nextjs-container-errors-pseudo-html-collapse] {\n    position: absolute;\n    left: 10px;\n    top: 10px;\n    color: inherit;\n    background: none;\n    border: none;\n    padding: 0;\n  }\n  [data-nextjs-container-errors-pseudo-html--diff-add] {\n    color: var(--color-ansi-green);\n  }\n  [data-nextjs-container-errors-pseudo-html--diff-remove] {\n    color: var(--color-ansi-red);\n  }\n  [data-nextjs-container-errors-pseudo-html--tag-error] {\n    color: var(--color-ansi-red);\n    font-weight: bold;\n  }\n  /* hide but text are still accessible in DOM */\n  [data-nextjs-container-errors-pseudo-html--hint] {\n    display: inline-block;\n    font-size: 0;\n  }\n  [data-nextjs-container-errors-pseudo-html--tag-adjacent='false'] {\n    color: var(--color-accents-1);\n  }\n"
    ]);
    _templateObject = function() {
        return data;
    };
    return data;
}
function RuntimeError(param) {
    let { error } = param;
    const { firstFrame, allLeadingFrames, allCallStackFrames } = _react.useMemo(()=>{
        const filteredFrames = error.frames.filter((f)=>{
            var _f_sourceStackFrame_file;
            return !(f.sourceStackFrame.file === "<anonymous>" && [
                "stringify",
                "<unknown>"
            ].includes(f.sourceStackFrame.methodName)) && !((_f_sourceStackFrame_file = f.sourceStackFrame.file) == null ? void 0 : _f_sourceStackFrame_file.startsWith("node:internal"));
        });
        const firstFirstPartyFrameIndex = filteredFrames.findIndex((entry)=>entry.expanded && Boolean(entry.originalCodeFrame) && Boolean(entry.originalStackFrame));
        var _filteredFrames_firstFirstPartyFrameIndex;
        return {
            firstFrame: (_filteredFrames_firstFirstPartyFrameIndex = filteredFrames[firstFirstPartyFrameIndex]) != null ? _filteredFrames_firstFirstPartyFrameIndex : null,
            allLeadingFrames: firstFirstPartyFrameIndex < 0 ? [] : filteredFrames.slice(0, firstFirstPartyFrameIndex),
            allCallStackFrames: filteredFrames.slice(firstFirstPartyFrameIndex + 1)
        };
    }, [
        error.frames
    ]);
    const [all, setAll] = _react.useState(firstFrame == null);
    const { canShowMore, leadingFramesGroupedByFramework, stackFramesGroupedByFramework } = _react.useMemo(()=>{
        const leadingFrames = allLeadingFrames.filter((f)=>f.expanded || all);
        const visibleCallStackFrames = allCallStackFrames.filter((f)=>f.expanded || all);
        return {
            canShowMore: allCallStackFrames.length !== visibleCallStackFrames.length || all && firstFrame != null,
            stackFramesGroupedByFramework: (0, _groupstackframesbyframework.groupStackFramesByFramework)(allCallStackFrames),
            leadingFramesGroupedByFramework: (0, _groupstackframesbyframework.groupStackFramesByFramework)(leadingFrames)
        };
    }, [
        all,
        allCallStackFrames,
        allLeadingFrames,
        firstFrame
    ]);
    return /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.Fragment, {
        children: [
            firstFrame ? /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.Fragment, {
                children: [
                    /*#__PURE__*/ (0, _jsxruntime.jsx)("h2", {
                        children: "Source"
                    }),
                    /*#__PURE__*/ (0, _jsxruntime.jsx)(_GroupedStackFrames.GroupedStackFrames, {
                        groupedStackFrames: leadingFramesGroupedByFramework,
                        show: all
                    }),
                    /*#__PURE__*/ (0, _jsxruntime.jsx)(_CodeFrame.CodeFrame, {
                        stackFrame: firstFrame.originalStackFrame,
                        codeFrame: firstFrame.originalCodeFrame
                    })
                ]
            }) : undefined,
            stackFramesGroupedByFramework.length ? /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.Fragment, {
                children: [
                    /*#__PURE__*/ (0, _jsxruntime.jsx)("h2", {
                        children: "Call Stack"
                    }),
                    /*#__PURE__*/ (0, _jsxruntime.jsx)(_GroupedStackFrames.GroupedStackFrames, {
                        groupedStackFrames: stackFramesGroupedByFramework,
                        show: all
                    })
                ]
            }) : undefined,
            canShowMore ? /*#__PURE__*/ (0, _jsxruntime.jsx)(_react.Fragment, {
                children: /*#__PURE__*/ (0, _jsxruntime.jsxs)("button", {
                    tabIndex: 10,
                    "data-nextjs-data-runtime-error-collapsed-action": true,
                    type: "button",
                    onClick: ()=>setAll(!all),
                    children: [
                        all ? "Hide" : "Show",
                        " collapsed frames"
                    ]
                })
            }) : undefined
        ]
    });
}
const styles = (0, _nooptemplate.noop)(_templateObject());

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=index.js.map