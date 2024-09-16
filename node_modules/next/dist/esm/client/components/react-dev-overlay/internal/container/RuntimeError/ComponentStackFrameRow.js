import { jsx as _jsx, jsxs as _jsxs, Fragment as _Fragment } from "react/jsx-runtime";
import React from "react";
import { useOpenInEditor } from "../../helpers/use-open-in-editor";
import { HotlinkedText } from "../../components/hot-linked-text";
function EditorLink(param) {
    let { children, componentStackFrame: { file, column, lineNumber } } = param;
    const open = useOpenInEditor({
        file,
        column,
        lineNumber
    });
    return /*#__PURE__*/ _jsxs("div", {
        tabIndex: 10,
        role: "link",
        onClick: open,
        title: "Click to open in your editor",
        children: [
            children,
            /*#__PURE__*/ _jsxs("svg", {
                xmlns: "http://www.w3.org/2000/svg",
                viewBox: "0 0 24 24",
                fill: "none",
                stroke: "currentColor",
                strokeWidth: "2",
                strokeLinecap: "round",
                strokeLinejoin: "round",
                children: [
                    /*#__PURE__*/ _jsx("path", {
                        d: "M18 13v6a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h6"
                    }),
                    /*#__PURE__*/ _jsx("polyline", {
                        points: "15 3 21 3 21 9"
                    }),
                    /*#__PURE__*/ _jsx("line", {
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
    return /*#__PURE__*/ _jsxs(_Fragment, {
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
        return /*#__PURE__*/ _jsx(EditorLink, {
            componentStackFrame: componentStackFrame,
            children: /*#__PURE__*/ _jsx("span", {
                children: /*#__PURE__*/ _jsx(LocationLine, {
                    componentStackFrame: componentStackFrame
                })
            })
        });
    }
    return /*#__PURE__*/ _jsx("div", {
        children: /*#__PURE__*/ _jsx(LocationLine, {
            componentStackFrame: componentStackFrame
        })
    });
}
export function ComponentStackFrameRow(param) {
    let { componentStackFrame } = param;
    const { component } = componentStackFrame;
    return /*#__PURE__*/ _jsxs("div", {
        "data-nextjs-component-stack-frame": true,
        children: [
            /*#__PURE__*/ _jsx("h3", {
                children: /*#__PURE__*/ _jsx(HotlinkedText, {
                    text: component
                })
            }),
            /*#__PURE__*/ _jsx(SourceLocation, {
                componentStackFrame: componentStackFrame
            })
        ]
    });
}

//# sourceMappingURL=ComponentStackFrameRow.js.map