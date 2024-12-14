"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    Errors: null,
    styles: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    Errors: function() {
        return Errors;
    },
    styles: function() {
        return styles;
    }
});
const _tagged_template_literal_loose = require("@swc/helpers/_/_tagged_template_literal_loose");
const _jsxruntime = require("react/jsx-runtime");
const _react = require("react");
const _shared = require("../../shared");
const _Dialog = require("../components/Dialog");
const _LeftRightDialogHeader = require("../components/LeftRightDialogHeader");
const _Overlay = require("../components/Overlay");
const _Toast = require("../components/Toast");
const _geterrorbytype = require("../helpers/get-error-by-type");
const _nooptemplate = require("../helpers/noop-template");
const _CloseIcon = require("../icons/CloseIcon");
const _RuntimeError = require("./RuntimeError");
const _VersionStalenessInfo = require("../components/VersionStalenessInfo");
const _errorsource = require("../../../../../shared/lib/error-source");
const _hotlinkedtext = require("../components/hot-linked-text");
const _componentstackpseudohtml = require("./RuntimeError/component-stack-pseudo-html");
const _hydrationerrorinfo = require("../helpers/hydration-error-info");
const _nodejsinspector = require("../components/nodejs-inspector");
const _copybutton = require("../components/copy-button");
const _consoleerror = require("../helpers/console-error");
function _templateObject() {
    const data = _tagged_template_literal_loose._([
        "\n  .nextjs-error-with-static {\n    bottom: calc(var(--size-gap-double) * 4.5);\n  }\n  .nextjs-container-errors-header {\n    position: relative;\n  }\n  .nextjs-container-errors-header > h1 {\n    font-size: var(--size-font-big);\n    line-height: var(--size-font-bigger);\n    font-weight: bold;\n    margin: calc(var(--size-gap-double) * 1.5) 0;\n    color: var(--color-title-h1);\n  }\n  .nextjs-container-errors-header small {\n    font-size: var(--size-font-small);\n    color: var(--color-accents-1);\n    margin-left: var(--size-gap-double);\n  }\n  .nextjs-container-errors-header small > span {\n    font-family: var(--font-stack-monospace);\n  }\n  .nextjs-container-errors-header p {\n    font-size: var(--size-font-small);\n    line-height: var(--size-font-big);\n    white-space: pre-wrap;\n  }\n  .nextjs__container_errors_desc {\n    font-family: var(--font-stack-monospace);\n    padding: var(--size-gap) var(--size-gap-double);\n    border-left: 2px solid var(--color-text-color-red-1);\n    margin-top: var(--size-gap);\n    font-weight: bold;\n    color: var(--color-text-color-red-1);\n    background-color: var(--color-text-background-red-1);\n  }\n  p.nextjs__container_errors__link {\n    margin: var(--size-gap-double) auto;\n    color: var(--color-text-color-red-1);\n    font-weight: 600;\n    font-size: 15px;\n  }\n  p.nextjs__container_errors__notes {\n    margin: var(--size-gap-double) auto;\n    color: var(--color-stack-notes);\n    font-weight: 600;\n    font-size: 15px;\n  }\n  .nextjs-container-errors-header > div > small {\n    margin: 0;\n    margin-top: var(--size-gap-half);\n  }\n  .nextjs-container-errors-header > p > a {\n    color: inherit;\n    font-weight: bold;\n  }\n  .nextjs-container-errors-body > h2:not(:first-child) {\n    margin-top: calc(var(--size-gap-double) + var(--size-gap));\n  }\n  .nextjs-container-errors-body > h2 {\n    color: var(--color-title-color);\n    margin-bottom: var(--size-gap);\n    font-size: var(--size-font-big);\n  }\n  .nextjs__container_errors__component-stack {\n    padding: 12px 32px;\n    color: var(--color-ansi-fg);\n    background: var(--color-ansi-bg);\n  }\n  .nextjs-toast-errors-parent {\n    cursor: pointer;\n    transition: transform 0.2s ease;\n  }\n  .nextjs-toast-errors-parent:hover {\n    transform: scale(1.1);\n  }\n  .nextjs-toast-errors {\n    display: flex;\n    align-items: center;\n    justify-content: flex-start;\n  }\n  .nextjs-toast-errors > svg {\n    margin-right: var(--size-gap);\n  }\n  .nextjs-toast-hide-button {\n    margin-left: var(--size-gap-triple);\n    border: none;\n    background: none;\n    color: var(--color-ansi-bright-white);\n    padding: 0;\n    transition: opacity 0.25s ease;\n    opacity: 0.7;\n  }\n  .nextjs-toast-hide-button:hover {\n    opacity: 1;\n  }\n  .nextjs-container-errors-header\n    > .nextjs-container-build-error-version-status {\n    position: absolute;\n    top: 0;\n    right: 0;\n  }\n  .nextjs__container_errors_inspect_copy_button {\n    cursor: pointer;\n    background: none;\n    border: none;\n    color: var(--color-ansi-bright-white);\n    font-size: 1.5rem;\n    padding: 0;\n    margin: 0;\n    margin-left: var(--size-gap);\n    transition: opacity 0.25s ease;\n  }\n  .nextjs__container_errors__error_title {\n    display: flex;\n    align-items: center;\n    justify-content: space-between;\n  }\n  .nextjs-data-runtime-error-inspect-link,\n  .nextjs-data-runtime-error-inspect-link:hover {\n    margin: 0 8px;\n    color: inherit;\n  }\n"
    ]);
    _templateObject = function() {
        return data;
    };
    return data;
}
function isNextjsLink(text) {
    return text.startsWith('https://nextjs.org');
}
function ErrorDescription(param) {
    let { error, hydrationWarning } = param;
    const isUnhandledOrReplayError = (0, _consoleerror.isUnhandledConsoleOrRejection)(error);
    const unhandledErrorType = isUnhandledOrReplayError ? (0, _consoleerror.getUnhandledErrorType)(error) : null;
    const isConsoleErrorStringMessage = unhandledErrorType === 'string';
    // If the error is:
    // - hydration warning
    // - captured console error or unhandled rejection
    // skip displaying the error name
    const title = isUnhandledOrReplayError && isConsoleErrorStringMessage || hydrationWarning ? '' : error.name + ': ';
    // If it's replayed error, display the environment name
    const environmentName = 'environmentName' in error ? error['environmentName'] : '';
    const envPrefix = environmentName ? "[ " + environmentName + " ] " : '';
    return /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
        children: [
            envPrefix,
            title,
            /*#__PURE__*/ (0, _jsxruntime.jsx)(_hotlinkedtext.HotlinkedText, {
                text: hydrationWarning || error.message,
                matcher: isNextjsLink
            })
        ]
    });
}
function getErrorSignature(ev) {
    const { event } = ev;
    switch(event.type){
        case _shared.ACTION_UNHANDLED_ERROR:
        case _shared.ACTION_UNHANDLED_REJECTION:
            {
                return event.reason.name + "::" + event.reason.message + "::" + event.reason.stack;
            }
        default:
            {}
    }
    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    const _ = event;
    return '';
}
function Errors(param) {
    let { isAppDir, errors, initialDisplayState, versionInfo, hasStaticIndicator, debugInfo } = param;
    var _activeError_componentStackFrames;
    const [lookups, setLookups] = (0, _react.useState)({});
    const [readyErrors, nextError] = (0, _react.useMemo)(()=>{
        let ready = [];
        let next = null;
        // Ensure errors are displayed in the order they occurred in:
        for(let idx = 0; idx < errors.length; ++idx){
            const e = errors[idx];
            const { id } = e;
            if (id in lookups) {
                ready.push(lookups[id]);
                continue;
            }
            // Check for duplicate errors
            if (idx > 0) {
                const prev = errors[idx - 1];
                if (getErrorSignature(prev) === getErrorSignature(e)) {
                    continue;
                }
            }
            next = e;
            break;
        }
        return [
            ready,
            next
        ];
    }, [
        errors,
        lookups
    ]);
    const isLoading = (0, _react.useMemo)(()=>{
        return readyErrors.length < 1 && Boolean(errors.length);
    }, [
        errors.length,
        readyErrors.length
    ]);
    (0, _react.useEffect)(()=>{
        if (nextError == null) {
            return;
        }
        let mounted = true;
        (0, _geterrorbytype.getErrorByType)(nextError, isAppDir).then((resolved)=>{
            // We don't care if the desired error changed while we were resolving,
            // thus we're not tracking it using a ref. Once the work has been done,
            // we'll store it.
            if (mounted) {
                setLookups((m)=>({
                        ...m,
                        [resolved.id]: resolved
                    }));
            }
        }, ()=>{
        // TODO: handle this, though an edge case
        });
        return ()=>{
            mounted = false;
        };
    }, [
        nextError,
        isAppDir
    ]);
    const [displayState, setDisplayState] = (0, _react.useState)(initialDisplayState);
    const [activeIdx, setActiveIndex] = (0, _react.useState)(0);
    const previous = (0, _react.useCallback)(()=>setActiveIndex((v)=>Math.max(0, v - 1)), []);
    const next = (0, _react.useCallback)(()=>setActiveIndex((v)=>Math.max(0, Math.min(readyErrors.length - 1, v + 1))), [
        readyErrors.length
    ]);
    const activeError = (0, _react.useMemo)(()=>{
        var _readyErrors_activeIdx;
        return (_readyErrors_activeIdx = readyErrors[activeIdx]) != null ? _readyErrors_activeIdx : null;
    }, [
        activeIdx,
        readyErrors
    ]);
    // Reset component state when there are no errors to be displayed.
    // This should never happen, but lets handle it.
    (0, _react.useEffect)(()=>{
        if (errors.length < 1) {
            setLookups({});
            setDisplayState('hidden');
            setActiveIndex(0);
        }
    }, [
        errors.length
    ]);
    const minimize = (0, _react.useCallback)(()=>setDisplayState('minimized'), []);
    const hide = (0, _react.useCallback)(()=>setDisplayState('hidden'), []);
    const fullscreen = (0, _react.useCallback)(()=>setDisplayState('fullscreen'), []);
    // This component shouldn't be rendered with no errors, but if it is, let's
    // handle it gracefully by rendering nothing.
    if (errors.length < 1 || activeError == null) {
        return null;
    }
    if (isLoading) {
        // TODO: better loading state
        return /*#__PURE__*/ (0, _jsxruntime.jsx)(_Overlay.Overlay, {});
    }
    if (displayState === 'hidden') {
        return null;
    }
    if (displayState === 'minimized') {
        return /*#__PURE__*/ (0, _jsxruntime.jsx)(_Toast.Toast, {
            "data-nextjs-toast": true,
            className: "nextjs-toast-errors-parent" + (hasStaticIndicator ? ' nextjs-error-with-static' : ''),
            onClick: fullscreen,
            children: /*#__PURE__*/ (0, _jsxruntime.jsxs)("div", {
                className: "nextjs-toast-errors",
                children: [
                    /*#__PURE__*/ (0, _jsxruntime.jsxs)("svg", {
                        xmlns: "http://www.w3.org/2000/svg",
                        width: "24",
                        height: "24",
                        viewBox: "0 0 24 24",
                        fill: "none",
                        stroke: "currentColor",
                        strokeWidth: "2",
                        strokeLinecap: "round",
                        strokeLinejoin: "round",
                        children: [
                            /*#__PURE__*/ (0, _jsxruntime.jsx)("circle", {
                                cx: "12",
                                cy: "12",
                                r: "10"
                            }),
                            /*#__PURE__*/ (0, _jsxruntime.jsx)("line", {
                                x1: "12",
                                y1: "8",
                                x2: "12",
                                y2: "12"
                            }),
                            /*#__PURE__*/ (0, _jsxruntime.jsx)("line", {
                                x1: "12",
                                y1: "16",
                                x2: "12.01",
                                y2: "16"
                            })
                        ]
                    }),
                    /*#__PURE__*/ (0, _jsxruntime.jsxs)("span", {
                        children: [
                            readyErrors.length,
                            " error",
                            readyErrors.length > 1 ? 's' : ''
                        ]
                    }),
                    /*#__PURE__*/ (0, _jsxruntime.jsx)("button", {
                        "data-nextjs-toast-errors-hide-button": true,
                        className: "nextjs-toast-hide-button",
                        type: "button",
                        onClick: (e)=>{
                            e.stopPropagation();
                            hide();
                        },
                        "aria-label": "Hide Errors",
                        children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_CloseIcon.CloseIcon, {})
                    })
                ]
            })
        });
    }
    const error = activeError.error;
    const isServerError = [
        'server',
        'edge-server'
    ].includes((0, _errorsource.getErrorSource)(error) || '');
    const isUnhandledError = (0, _consoleerror.isUnhandledConsoleOrRejection)(error);
    const errorDetails = error.details || {};
    const notes = errorDetails.notes || '';
    const [warningTemplate, serverContent, clientContent] = errorDetails.warning || [
        null,
        '',
        ''
    ];
    const hydrationErrorType = (0, _hydrationerrorinfo.getHydrationWarningType)(warningTemplate);
    const hydrationWarning = warningTemplate ? warningTemplate.replace('%s', serverContent).replace('%s', clientContent).replace('%s', '') // remove the %s for stack
    .replace(/%s$/, '') // If there's still a %s at the end, remove it
    .replace(/^Warning: /, '').replace(/^Error: /, '') : null;
    return /*#__PURE__*/ (0, _jsxruntime.jsx)(_Overlay.Overlay, {
        children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_Dialog.Dialog, {
            type: "error",
            "aria-labelledby": "nextjs__container_errors_label",
            "aria-describedby": "nextjs__container_errors_desc",
            onClose: isServerError ? undefined : minimize,
            children: /*#__PURE__*/ (0, _jsxruntime.jsxs)(_Dialog.DialogContent, {
                children: [
                    /*#__PURE__*/ (0, _jsxruntime.jsxs)(_Dialog.DialogHeader, {
                        className: "nextjs-container-errors-header",
                        children: [
                            /*#__PURE__*/ (0, _jsxruntime.jsxs)(_LeftRightDialogHeader.LeftRightDialogHeader, {
                                previous: activeIdx > 0 ? previous : null,
                                next: activeIdx < readyErrors.length - 1 ? next : null,
                                close: isServerError ? undefined : minimize,
                                children: [
                                    /*#__PURE__*/ (0, _jsxruntime.jsxs)("small", {
                                        children: [
                                            /*#__PURE__*/ (0, _jsxruntime.jsx)("span", {
                                                children: activeIdx + 1
                                            }),
                                            " of",
                                            ' ',
                                            /*#__PURE__*/ (0, _jsxruntime.jsx)("span", {
                                                "data-nextjs-dialog-header-total-count": true,
                                                children: readyErrors.length
                                            }),
                                            ' error',
                                            readyErrors.length < 2 ? '' : 's'
                                        ]
                                    }),
                                    /*#__PURE__*/ (0, _jsxruntime.jsx)(_VersionStalenessInfo.VersionStalenessInfo, {
                                        versionInfo: versionInfo
                                    })
                                ]
                            }),
                            /*#__PURE__*/ (0, _jsxruntime.jsxs)("div", {
                                className: "nextjs__container_errors__error_title",
                                children: [
                                    /*#__PURE__*/ (0, _jsxruntime.jsx)("h1", {
                                        id: "nextjs__container_errors_label",
                                        className: "nextjs__container_errors_label",
                                        children: isServerError ? 'Server Error' : isUnhandledError ? 'Console Error' : 'Unhandled Runtime Error'
                                    }),
                                    /*#__PURE__*/ (0, _jsxruntime.jsxs)("span", {
                                        children: [
                                            /*#__PURE__*/ (0, _jsxruntime.jsx)(_copybutton.CopyButton, {
                                                "data-nextjs-data-runtime-error-copy-stack": true,
                                                actionLabel: "Copy error stack",
                                                successLabel: "Copied",
                                                content: error.stack || '',
                                                disabled: !error.stack
                                            }),
                                            /*#__PURE__*/ (0, _jsxruntime.jsx)(_nodejsinspector.NodejsInspectorCopyButton, {
                                                devtoolsFrontendUrl: debugInfo == null ? void 0 : debugInfo.devtoolsFrontendUrl
                                            })
                                        ]
                                    })
                                ]
                            }),
                            /*#__PURE__*/ (0, _jsxruntime.jsx)("p", {
                                id: "nextjs__container_errors_desc",
                                className: "nextjs__container_errors_desc",
                                children: /*#__PURE__*/ (0, _jsxruntime.jsx)(ErrorDescription, {
                                    error: error,
                                    hydrationWarning: hydrationWarning
                                })
                            }),
                            notes ? /*#__PURE__*/ (0, _jsxruntime.jsx)(_jsxruntime.Fragment, {
                                children: /*#__PURE__*/ (0, _jsxruntime.jsx)("p", {
                                    id: "nextjs__container_errors__notes",
                                    className: "nextjs__container_errors__notes",
                                    children: notes
                                })
                            }) : null,
                            hydrationWarning ? /*#__PURE__*/ (0, _jsxruntime.jsx)("p", {
                                id: "nextjs__container_errors__link",
                                className: "nextjs__container_errors__link",
                                children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_hotlinkedtext.HotlinkedText, {
                                    text: "See more info here: https://nextjs.org/docs/messages/react-hydration-error"
                                })
                            }) : null,
                            hydrationWarning && (((_activeError_componentStackFrames = activeError.componentStackFrames) == null ? void 0 : _activeError_componentStackFrames.length) || !!errorDetails.reactOutputComponentDiff) ? /*#__PURE__*/ (0, _jsxruntime.jsx)(_componentstackpseudohtml.PseudoHtmlDiff, {
                                className: "nextjs__container_errors__component-stack",
                                hydrationMismatchType: hydrationErrorType,
                                componentStackFrames: activeError.componentStackFrames || [],
                                firstContent: serverContent,
                                secondContent: clientContent,
                                reactOutputComponentDiff: errorDetails.reactOutputComponentDiff
                            }) : null,
                            isServerError ? /*#__PURE__*/ (0, _jsxruntime.jsx)("div", {
                                children: /*#__PURE__*/ (0, _jsxruntime.jsx)("small", {
                                    children: "This error happened while generating the page. Any console logs will be displayed in the terminal window."
                                })
                            }) : undefined
                        ]
                    }),
                    /*#__PURE__*/ (0, _jsxruntime.jsx)(_Dialog.DialogBody, {
                        className: "nextjs-container-errors-body",
                        children: /*#__PURE__*/ (0, _jsxruntime.jsx)(_RuntimeError.RuntimeError, {
                            error: activeError
                        }, activeError.id.toString())
                    })
                ]
            })
        })
    });
}
const styles = (0, _nooptemplate.noop)(_templateObject());

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=Errors.js.map