import { _ as _tagged_template_literal_loose } from "@swc/helpers/_/_tagged_template_literal_loose";
function _templateObject() {
    const data = _tagged_template_literal_loose([
        "\n  button[data-nextjs-data-runtime-error-collapsed-action] {\n    background: none;\n    border: none;\n    padding: 0;\n    font-size: var(--size-font-small);\n    line-height: var(--size-font-bigger);\n    color: var(--color-accents-3);\n  }\n\n  [data-nextjs-call-stack-frame]:not(:last-child),\n  [data-nextjs-component-stack-frame]:not(:last-child) {\n    margin-bottom: var(--size-gap-double);\n  }\n\n  [data-nextjs-call-stack-frame] > h3,\n  [data-nextjs-component-stack-frame] > h3 {\n    margin-top: 0;\n    margin-bottom: var(--size-gap);\n    font-family: var(--font-stack-monospace);\n    font-size: var(--size-font);\n    color: #222;\n  }\n  [data-nextjs-call-stack-frame] > h3[data-nextjs-frame-expanded='false'] {\n    color: #666;\n  }\n  [data-nextjs-call-stack-frame] > div,\n  [data-nextjs-component-stack-frame] > div {\n    display: flex;\n    align-items: center;\n    padding-left: calc(var(--size-gap) + var(--size-gap-half));\n    font-size: var(--size-font-small);\n    color: #999;\n  }\n  [data-nextjs-call-stack-frame] > div > svg,\n  [data-nextjs-component-stack-frame] > [role='link'] > svg {\n    width: auto;\n    height: var(--size-font-small);\n    margin-left: var(--size-gap);\n    flex-shrink: 0;\n\n    display: none;\n  }\n\n  [data-nextjs-call-stack-frame] > div[data-has-source],\n  [data-nextjs-component-stack-frame] > [role='link'] {\n    cursor: pointer;\n  }\n  [data-nextjs-call-stack-frame] > div[data-has-source]:hover,\n  [data-nextjs-component-stack-frame] > [role='link']:hover {\n    text-decoration: underline dotted;\n  }\n  [data-nextjs-call-stack-frame] > div[data-has-source] > svg,\n  [data-nextjs-component-stack-frame] > [role='link'] > svg {\n    display: unset;\n  }\n\n  [data-nextjs-call-stack-framework-icon] {\n    margin-right: var(--size-gap);\n  }\n  [data-nextjs-call-stack-framework-icon='next'] > mask {\n    mask-type: alpha;\n  }\n  [data-nextjs-call-stack-framework-icon='react'] {\n    color: rgb(20, 158, 202);\n  }\n  [data-nextjs-collapsed-call-stack-details][open]\n    [data-nextjs-call-stack-chevron-icon] {\n    transform: rotate(90deg);\n  }\n  [data-nextjs-collapsed-call-stack-details] summary {\n    display: flex;\n    align-items: center;\n    margin-bottom: var(--size-gap);\n    list-style: none;\n  }\n  [data-nextjs-collapsed-call-stack-details] summary::-webkit-details-marker {\n    display: none;\n  }\n\n  [data-nextjs-collapsed-call-stack-details] h3 {\n    color: #666;\n  }\n  [data-nextjs-collapsed-call-stack-details] [data-nextjs-call-stack-frame] {\n    margin-bottom: var(--size-gap-double);\n  }\n\n  [data-nextjs-container-errors-pseudo-html] {\n    position: relative;\n  }\n  [data-nextjs-container-errors-pseudo-html-collapse] {\n    position: absolute;\n    left: 10px;\n    top: 10px;\n    color: inherit;\n    background: none;\n    border: none;\n    padding: 0;\n  }\n  [data-nextjs-container-errors-pseudo-html--diff-add] {\n    color: var(--color-ansi-green);\n  }\n  [data-nextjs-container-errors-pseudo-html--diff-remove] {\n    color: var(--color-ansi-red);\n  }\n  [data-nextjs-container-errors-pseudo-html--tag-error] {\n    color: var(--color-ansi-red);\n    font-weight: bold;\n  }\n  /* hide but text are still accessible in DOM */\n  [data-nextjs-container-errors-pseudo-html--hint] {\n    display: inline-block;\n    font-size: 0;\n  }\n  [data-nextjs-container-errors-pseudo-html--tag-adjacent='false'] {\n    color: var(--color-accents-1);\n  }\n"
    ]);
    _templateObject = function() {
        return data;
    };
    return data;
}
import { jsx as _jsx, jsxs as _jsxs } from "react/jsx-runtime";
import * as React from "react";
import { CodeFrame } from "../../components/CodeFrame";
import { noop as css } from "../../helpers/noop-template";
import { groupStackFramesByFramework } from "../../helpers/group-stack-frames-by-framework";
import { GroupedStackFrames } from "./GroupedStackFrames";
export function RuntimeError(param) {
    let { error } = param;
    const { firstFrame, allLeadingFrames, allCallStackFrames } = React.useMemo(()=>{
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
    const [all, setAll] = React.useState(firstFrame == null);
    const { canShowMore, leadingFramesGroupedByFramework, stackFramesGroupedByFramework } = React.useMemo(()=>{
        const leadingFrames = allLeadingFrames.filter((f)=>f.expanded || all);
        const visibleCallStackFrames = allCallStackFrames.filter((f)=>f.expanded || all);
        return {
            canShowMore: allCallStackFrames.length !== visibleCallStackFrames.length || all && firstFrame != null,
            stackFramesGroupedByFramework: groupStackFramesByFramework(allCallStackFrames),
            leadingFramesGroupedByFramework: groupStackFramesByFramework(leadingFrames)
        };
    }, [
        all,
        allCallStackFrames,
        allLeadingFrames,
        firstFrame
    ]);
    return /*#__PURE__*/ _jsxs(React.Fragment, {
        children: [
            firstFrame ? /*#__PURE__*/ _jsxs(React.Fragment, {
                children: [
                    /*#__PURE__*/ _jsx("h2", {
                        children: "Source"
                    }),
                    /*#__PURE__*/ _jsx(GroupedStackFrames, {
                        groupedStackFrames: leadingFramesGroupedByFramework,
                        show: all
                    }),
                    /*#__PURE__*/ _jsx(CodeFrame, {
                        stackFrame: firstFrame.originalStackFrame,
                        codeFrame: firstFrame.originalCodeFrame
                    })
                ]
            }) : undefined,
            stackFramesGroupedByFramework.length ? /*#__PURE__*/ _jsxs(React.Fragment, {
                children: [
                    /*#__PURE__*/ _jsx("h2", {
                        children: "Call Stack"
                    }),
                    /*#__PURE__*/ _jsx(GroupedStackFrames, {
                        groupedStackFrames: stackFramesGroupedByFramework,
                        show: all
                    })
                ]
            }) : undefined,
            canShowMore ? /*#__PURE__*/ _jsx(React.Fragment, {
                children: /*#__PURE__*/ _jsxs("button", {
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
export const styles = css(_templateObject());

//# sourceMappingURL=index.js.map