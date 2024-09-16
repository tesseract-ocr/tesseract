import { jsx as _jsx, jsxs as _jsxs } from "react/jsx-runtime";
import Anser from "next/dist/compiled/anser";
import * as React from "react";
import stripAnsi from "next/dist/compiled/strip-ansi";
import { getFrameSource } from "../../helpers/stack-frame";
import { useOpenInEditor } from "../../helpers/use-open-in-editor";
import { HotlinkedText } from "../hot-linked-text";
export const CodeFrame = function CodeFrame(param) {
    let { stackFrame, codeFrame } = param;
    // Strip leading spaces out of the code frame:
    const formattedFrame = React.useMemo(()=>{
        const lines = codeFrame.split(/\r?\n/g);
        // Find the minimum length of leading spaces after `|` in the code frame
        const miniLeadingSpacesLength = lines.map((line)=>/^>? +\d+ +\| [ ]+/.exec(stripAnsi(line)) === null ? null : /^>? +\d+ +\| ( *)/.exec(stripAnsi(line))).filter(Boolean).map((v)=>v.pop()).reduce((c, n)=>isNaN(c) ? n.length : Math.min(c, n.length), NaN);
        // When the minimum length of leading spaces is greater than 1, remove them
        // from the code frame to help the indentation looks better when there's a lot leading spaces.
        if (miniLeadingSpacesLength > 1) {
            return lines.map((line, a)=>~(a = line.indexOf("|")) ? line.substring(0, a) + line.substring(a).replace("^\\ {" + miniLeadingSpacesLength + "}", "") : line).join("\n");
        }
        return lines.join("\n");
    }, [
        codeFrame
    ]);
    const decoded = React.useMemo(()=>{
        return Anser.ansiToJson(formattedFrame, {
            json: true,
            use_classes: true,
            remove_empty: true
        });
    }, [
        formattedFrame
    ]);
    const open = useOpenInEditor({
        file: stackFrame.file,
        lineNumber: stackFrame.lineNumber,
        column: stackFrame.column
    });
    // TODO: make the caret absolute
    return /*#__PURE__*/ _jsxs("div", {
        "data-nextjs-codeframe": true,
        children: [
            /*#__PURE__*/ _jsx("div", {
                children: /*#__PURE__*/ _jsxs("p", {
                    role: "link",
                    onClick: open,
                    tabIndex: 1,
                    title: "Click to open in your editor",
                    children: [
                        /*#__PURE__*/ _jsxs("span", {
                            children: [
                                getFrameSource(stackFrame),
                                " @",
                                " ",
                                /*#__PURE__*/ _jsx(HotlinkedText, {
                                    text: stackFrame.methodName
                                })
                            ]
                        }),
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
                })
            }),
            /*#__PURE__*/ _jsx("pre", {
                children: decoded.map((entry, index)=>/*#__PURE__*/ _jsx("span", {
                        style: {
                            color: entry.fg ? "var(--color-" + entry.fg + ")" : undefined,
                            ...entry.decoration === "bold" ? {
                                fontWeight: 800
                            } : entry.decoration === "italic" ? {
                                fontStyle: "italic"
                            } : undefined
                        },
                        children: entry.content
                    }, "frame-" + index))
            })
        ]
    });
};

//# sourceMappingURL=CodeFrame.js.map