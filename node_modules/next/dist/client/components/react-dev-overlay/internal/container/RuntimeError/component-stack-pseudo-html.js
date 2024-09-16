"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "PseudoHtmlDiff", {
    enumerable: true,
    get: function() {
        return PseudoHtmlDiff;
    }
});
const _jsxruntime = require("react/jsx-runtime");
const _react = require("react");
const _CollapseIcon = require("../../icons/CollapseIcon");
function getAdjacentProps(isAdj) {
    return {
        "data-nextjs-container-errors-pseudo-html--tag-adjacent": isAdj
    };
}
function PseudoHtmlDiff(param) {
    let { componentStackFrames, firstContent, secondContent, hydrationMismatchType, ...props } = param;
    const isHtmlTagsWarning = hydrationMismatchType === "tag";
    // For text mismatch, mismatched text will take 2 rows, so we display 4 rows of component stack
    const MAX_NON_COLLAPSED_FRAMES = isHtmlTagsWarning ? 6 : 4;
    const shouldCollapse = componentStackFrames.length > MAX_NON_COLLAPSED_FRAMES;
    const [isHtmlCollapsed, toggleCollapseHtml] = (0, _react.useState)(shouldCollapse);
    const htmlComponents = (0, _react.useMemo)(()=>{
        const tagNames = isHtmlTagsWarning ? [
            firstContent.replace(/<|>/g, ""),
            secondContent.replace(/<|>/g, "")
        ] : [];
        const nestedHtmlStack = [];
        let lastText = "";
        const componentStack = componentStackFrames.map((frame)=>frame.component).reverse();
        // [child index, parent index]
        const matchedIndex = [
            -1,
            -1
        ];
        if (isHtmlTagsWarning) {
            // Reverse search for the child tag
            for(let i = componentStack.length - 1; i >= 0; i--){
                if (componentStack[i] === tagNames[0]) {
                    matchedIndex[0] = i;
                    break;
                }
            }
            // Start searching parent tag from child tag above
            for(let i = matchedIndex[0] - 1; i >= 0; i--){
                if (componentStack[i] === tagNames[1]) {
                    matchedIndex[1] = i;
                    break;
                }
            }
        }
        componentStack.forEach((component, index, componentList)=>{
            const spaces = " ".repeat(nestedHtmlStack.length * 2);
            // const prevComponent = componentList[index - 1]
            // const nextComponent = componentList[index + 1]
            // When component is the server or client tag name, highlight it
            const isHighlightedTag = isHtmlTagsWarning ? index === matchedIndex[0] || index === matchedIndex[1] : tagNames.includes(component);
            const isAdjacentTag = isHighlightedTag || Math.abs(index - matchedIndex[0]) <= 1 || Math.abs(index - matchedIndex[1]) <= 1;
            const isLastFewFrames = !isHtmlTagsWarning && index >= componentList.length - 6;
            const adjProps = getAdjacentProps(isAdjacentTag);
            if (isHtmlTagsWarning && isAdjacentTag || isLastFewFrames) {
                const codeLine = /*#__PURE__*/ (0, _jsxruntime.jsxs)("span", {
                    children: [
                        spaces,
                        /*#__PURE__*/ (0, _jsxruntime.jsx)("span", {
                            ...adjProps,
                            ...isHighlightedTag ? {
                                "data-nextjs-container-errors-pseudo-html--tag-error": true
                            } : undefined,
                            children: "<" + component + ">\n"
                        })
                    ]
                });
                lastText = component;
                const wrappedCodeLine = /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.Fragment, {
                    children: [
                        codeLine,
                        isHighlightedTag && /*#__PURE__*/ (0, _jsxruntime.jsx)("span", {
                            "data-nextjs-container-errors-pseudo-html--hint": true,
                            children: spaces + "^".repeat(component.length + 2) + "\n"
                        })
                    ]
                }, nestedHtmlStack.length);
                nestedHtmlStack.push(wrappedCodeLine);
            } else {
                if (nestedHtmlStack.length >= MAX_NON_COLLAPSED_FRAMES && isHtmlCollapsed) {
                    return;
                }
                if (!isHtmlCollapsed || isLastFewFrames) {
                    nestedHtmlStack.push(/*#__PURE__*/ (0, _react.createElement)("span", {
                        ...adjProps,
                        key: nestedHtmlStack.length,
                        children: [
                            spaces,
                            "<" + component + ">\n"
                        ]
                    }));
                } else if (isHtmlCollapsed && lastText !== "...") {
                    lastText = "...";
                    nestedHtmlStack.push(/*#__PURE__*/ (0, _react.createElement)("span", {
                        ...adjProps,
                        key: nestedHtmlStack.length,
                        children: [
                            spaces,
                            "...\n"
                        ]
                    }));
                }
            }
        });
        // Hydration mismatch: text or text-tag
        if (!isHtmlTagsWarning) {
            const spaces = " ".repeat(nestedHtmlStack.length * 2);
            let wrappedCodeLine;
            if (hydrationMismatchType === "text") {
                // hydration type is "text", represent [server content, client content]
                wrappedCodeLine = /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.Fragment, {
                    children: [
                        /*#__PURE__*/ (0, _jsxruntime.jsx)("span", {
                            "data-nextjs-container-errors-pseudo-html--diff-remove": true,
                            children: spaces + ('"' + firstContent + '"\n')
                        }),
                        /*#__PURE__*/ (0, _jsxruntime.jsx)("span", {
                            "data-nextjs-container-errors-pseudo-html--diff-add": true,
                            children: spaces + ('"' + secondContent + '"\n')
                        })
                    ]
                }, nestedHtmlStack.length);
            } else {
                // hydration type is "text-in-tag", represent [parent tag, mismatch content]
                wrappedCodeLine = /*#__PURE__*/ (0, _jsxruntime.jsxs)(_react.Fragment, {
                    children: [
                        /*#__PURE__*/ (0, _jsxruntime.jsx)("span", {
                            "data-nextjs-container-errors-pseudo-html--tag-adjacent": true,
                            children: spaces + ("<" + secondContent + ">\n")
                        }),
                        /*#__PURE__*/ (0, _jsxruntime.jsx)("span", {
                            "data-nextjs-container-errors-pseudo-html--diff-remove": true,
                            children: spaces + ('  "' + firstContent + '"\n')
                        })
                    ]
                }, nestedHtmlStack.length);
            }
            nestedHtmlStack.push(wrappedCodeLine);
        }
        return nestedHtmlStack;
    }, [
        componentStackFrames,
        isHtmlCollapsed,
        firstContent,
        secondContent,
        isHtmlTagsWarning,
        hydrationMismatchType,
        MAX_NON_COLLAPSED_FRAMES
    ]);
    return /*#__PURE__*/ (0, _jsxruntime.jsxs)("div", {
        "data-nextjs-container-errors-pseudo-html": true,
        children: [
            /*#__PURE__*/ (0, _jsxruntime.jsx)("button", {
                tabIndex: 10,
                "data-nextjs-container-errors-pseudo-html-collapse": true,
                onClick: ()=>toggleCollapseHtml(!isHtmlCollapsed),
                children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_CollapseIcon.CollapseIcon, {
                    collapsed: isHtmlCollapsed
                })
            }),
            /*#__PURE__*/ (0, _jsxruntime.jsx)("pre", {
                ...props,
                children: /*#__PURE__*/ (0, _jsxruntime.jsx)("code", {
                    children: htmlComponents
                })
            })
        ]
    });
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=component-stack-pseudo-html.js.map