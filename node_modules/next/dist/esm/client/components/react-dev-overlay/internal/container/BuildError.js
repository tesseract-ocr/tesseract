import { _ as _tagged_template_literal_loose } from "@swc/helpers/_/_tagged_template_literal_loose";
function _templateObject() {
    const data = _tagged_template_literal_loose([
        "\n  h1.nextjs__container_errors_label {\n    font-size: var(--size-font-big);\n    line-height: var(--size-font-bigger);\n    font-weight: bold;\n    margin: var(--size-gap-double) 0;\n  }\n  .nextjs-container-errors-header p {\n    font-size: var(--size-font-small);\n    line-height: var(--size-font-big);\n    white-space: pre-wrap;\n  }\n  .nextjs-container-errors-body footer {\n    margin-top: var(--size-gap);\n  }\n  .nextjs-container-errors-body footer p {\n    margin: 0;\n  }\n\n  .nextjs-container-errors-body small {\n    color: var(--color-font);\n  }\n"
    ]);
    _templateObject = function() {
        return data;
    };
    return data;
}
import { jsx as _jsx, jsxs as _jsxs } from "react/jsx-runtime";
import * as React from 'react';
import { Dialog, DialogBody, DialogContent, DialogHeader } from '../components/Dialog';
import { Overlay } from '../components/Overlay';
import { Terminal } from '../components/Terminal';
import { VersionStalenessInfo } from '../components/VersionStalenessInfo';
import { noop as css } from '../helpers/noop-template';
export const BuildError = function BuildError(param) {
    let { message, versionInfo } = param;
    const noop = React.useCallback(()=>{}, []);
    return /*#__PURE__*/ _jsx(Overlay, {
        fixed: true,
        children: /*#__PURE__*/ _jsx(Dialog, {
            type: "error",
            "aria-labelledby": "nextjs__container_error_label",
            "aria-describedby": "nextjs__container_error_desc",
            onClose: noop,
            children: /*#__PURE__*/ _jsxs(DialogContent, {
                children: [
                    /*#__PURE__*/ _jsxs(DialogHeader, {
                        className: "nextjs-container-errors-header",
                        children: [
                            /*#__PURE__*/ _jsx("h1", {
                                id: "nextjs__container_errors_label",
                                className: "nextjs__container_errors_label",
                                children: 'Build Error'
                            }),
                            /*#__PURE__*/ _jsx(VersionStalenessInfo, {
                                versionInfo: versionInfo
                            }),
                            /*#__PURE__*/ _jsx("p", {
                                id: "nextjs__container_errors_desc",
                                className: "nextjs__container_errors_desc",
                                children: "Failed to compile"
                            })
                        ]
                    }),
                    /*#__PURE__*/ _jsxs(DialogBody, {
                        className: "nextjs-container-errors-body",
                        children: [
                            /*#__PURE__*/ _jsx(Terminal, {
                                content: message
                            }),
                            /*#__PURE__*/ _jsx("footer", {
                                children: /*#__PURE__*/ _jsx("p", {
                                    id: "nextjs__container_build_error_desc",
                                    children: /*#__PURE__*/ _jsx("small", {
                                        children: "This error occurred during the build process and can only be dismissed by fixing the error."
                                    })
                                })
                            })
                        ]
                    })
                ]
            })
        })
    });
};
export const styles = css(_templateObject());

//# sourceMappingURL=BuildError.js.map