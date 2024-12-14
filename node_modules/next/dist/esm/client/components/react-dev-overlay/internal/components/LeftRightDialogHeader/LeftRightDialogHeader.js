import { jsx as _jsx, jsxs as _jsxs } from "react/jsx-runtime";
import * as React from 'react';
import { CloseIcon } from '../../icons/CloseIcon';
const LeftRightDialogHeader = function LeftRightDialogHeader(param) {
    let { children, className, previous, next, close } = param;
    const buttonLeft = React.useRef(null);
    const buttonRight = React.useRef(null);
    const buttonClose = React.useRef(null);
    const [nav, setNav] = React.useState(null);
    const onNav = React.useCallback((el)=>{
        setNav(el);
    }, []);
    React.useEffect(()=>{
        if (nav == null) {
            return;
        }
        const root = nav.getRootNode();
        const d = self.document;
        function handler(e) {
            if (e.key === 'ArrowLeft') {
                e.preventDefault();
                e.stopPropagation();
                if (buttonLeft.current) {
                    buttonLeft.current.focus();
                }
                previous && previous();
            } else if (e.key === 'ArrowRight') {
                e.preventDefault();
                e.stopPropagation();
                if (buttonRight.current) {
                    buttonRight.current.focus();
                }
                next && next();
            } else if (e.key === 'Escape') {
                e.preventDefault();
                e.stopPropagation();
                if (root instanceof ShadowRoot) {
                    const a = root.activeElement;
                    if (a && a !== buttonClose.current && a instanceof HTMLElement) {
                        a.blur();
                        return;
                    }
                }
                close == null ? void 0 : close();
            }
        }
        root.addEventListener('keydown', handler);
        if (root !== d) {
            d.addEventListener('keydown', handler);
        }
        return function() {
            root.removeEventListener('keydown', handler);
            if (root !== d) {
                d.removeEventListener('keydown', handler);
            }
        };
    }, [
        close,
        nav,
        next,
        previous
    ]);
    // Unlock focus for browsers like Firefox, that break all user focus if the
    // currently focused item becomes disabled.
    React.useEffect(()=>{
        if (nav == null) {
            return;
        }
        const root = nav.getRootNode();
        // Always true, but we do this for TypeScript:
        if (root instanceof ShadowRoot) {
            const a = root.activeElement;
            if (previous == null) {
                if (buttonLeft.current && a === buttonLeft.current) {
                    buttonLeft.current.blur();
                }
            } else if (next == null) {
                if (buttonRight.current && a === buttonRight.current) {
                    buttonRight.current.blur();
                }
            }
        }
    }, [
        nav,
        next,
        previous
    ]);
    return /*#__PURE__*/ _jsxs("div", {
        "data-nextjs-dialog-left-right": true,
        className: className,
        children: [
            /*#__PURE__*/ _jsxs("nav", {
                ref: onNav,
                children: [
                    /*#__PURE__*/ _jsx("button", {
                        ref: buttonLeft,
                        type: "button",
                        disabled: previous == null ? true : undefined,
                        "aria-disabled": previous == null ? true : undefined,
                        onClick: previous != null ? previous : undefined,
                        children: /*#__PURE__*/ _jsxs("svg", {
                            viewBox: "0 0 14 14",
                            fill: "none",
                            xmlns: "http://www.w3.org/2000/svg",
                            children: [
                                /*#__PURE__*/ _jsx("title", {
                                    children: "previous"
                                }),
                                /*#__PURE__*/ _jsx("path", {
                                    d: "M6.99996 1.16666L1.16663 6.99999L6.99996 12.8333M12.8333 6.99999H1.99996H12.8333Z",
                                    stroke: "currentColor",
                                    strokeWidth: "2",
                                    strokeLinecap: "round",
                                    strokeLinejoin: "round"
                                })
                            ]
                        })
                    }),
                    /*#__PURE__*/ _jsx("button", {
                        ref: buttonRight,
                        type: "button",
                        disabled: next == null ? true : undefined,
                        "aria-disabled": next == null ? true : undefined,
                        onClick: next != null ? next : undefined,
                        children: /*#__PURE__*/ _jsxs("svg", {
                            viewBox: "0 0 14 14",
                            fill: "none",
                            xmlns: "http://www.w3.org/2000/svg",
                            children: [
                                /*#__PURE__*/ _jsx("title", {
                                    children: "next"
                                }),
                                /*#__PURE__*/ _jsx("path", {
                                    d: "M6.99996 1.16666L12.8333 6.99999L6.99996 12.8333M1.16663 6.99999H12H1.16663Z",
                                    stroke: "currentColor",
                                    strokeWidth: "2",
                                    strokeLinecap: "round",
                                    strokeLinejoin: "round"
                                })
                            ]
                        })
                    }),
                    children
                ]
            }),
            close ? /*#__PURE__*/ _jsx("button", {
                "data-nextjs-errors-dialog-left-right-close-button": true,
                ref: buttonClose,
                type: "button",
                onClick: close,
                "aria-label": "Close",
                children: /*#__PURE__*/ _jsx("span", {
                    "aria-hidden": "true",
                    children: /*#__PURE__*/ _jsx(CloseIcon, {})
                })
            }) : null
        ]
    });
};
export { LeftRightDialogHeader };

//# sourceMappingURL=LeftRightDialogHeader.js.map