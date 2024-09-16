import { jsx as _jsx, jsxs as _jsxs, Fragment as _Fragment } from "react/jsx-runtime";
import { CallStackFrame } from "./CallStackFrame";
import { CollapseIcon } from "../../icons/CollapseIcon";
import { FrameworkIcon } from "../../icons/FrameworkIcon";
function FrameworkGroup(param) {
    let { framework, stackFrames } = param;
    return /*#__PURE__*/ _jsxs("details", {
        "data-nextjs-collapsed-call-stack-details": true,
        children: [
            /*#__PURE__*/ _jsxs("summary", {
                tabIndex: 10,
                children: [
                    /*#__PURE__*/ _jsx(CollapseIcon, {}),
                    /*#__PURE__*/ _jsx(FrameworkIcon, {
                        framework: framework
                    }),
                    framework === "react" ? "React" : "Next.js"
                ]
            }),
            stackFrames.map((frame, index)=>/*#__PURE__*/ _jsx(CallStackFrame, {
                    frame: frame
                }, "call-stack-" + index))
        ]
    });
}
export function GroupedStackFrames(param) {
    let { groupedStackFrames, show } = param;
    if (!show) return;
    return /*#__PURE__*/ _jsx(_Fragment, {
        children: groupedStackFrames.map((stackFramesGroup, groupIndex)=>{
            // Collapse React and Next.js frames
            if (stackFramesGroup.framework) {
                return /*#__PURE__*/ _jsx(FrameworkGroup, {
                    framework: stackFramesGroup.framework,
                    stackFrames: stackFramesGroup.stackFrames
                }, "call-stack-framework-group-" + groupIndex);
            }
            return(// Don't group non React and Next.js frames
            stackFramesGroup.stackFrames.map((frame, frameIndex)=>/*#__PURE__*/ _jsx(CallStackFrame, {
                    frame: frame
                }, "call-stack-" + groupIndex + "-" + frameIndex)));
        })
    });
}

//# sourceMappingURL=GroupedStackFrames.js.map