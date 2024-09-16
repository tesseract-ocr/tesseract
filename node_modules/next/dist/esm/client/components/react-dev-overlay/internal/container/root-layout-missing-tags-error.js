import { jsx as _jsx, jsxs as _jsxs } from "react/jsx-runtime";
import * as React from "react";
import { Dialog, DialogContent, DialogHeader } from "../components/Dialog";
import { Overlay } from "../components/Overlay";
import { VersionStalenessInfo } from "../components/VersionStalenessInfo";
import { HotlinkedText } from "../components/hot-linked-text";
export const RootLayoutMissingTagsError = function RootLayoutMissingTagsError(param) {
    let { missingTags, versionInfo } = param;
    const noop = React.useCallback(()=>{}, []);
    return /*#__PURE__*/ _jsx(Overlay, {
        children: /*#__PURE__*/ _jsx(Dialog, {
            type: "error",
            "aria-labelledby": "nextjs__container_errors_label",
            "aria-describedby": "nextjs__container_errors_desc",
            onClose: noop,
            children: /*#__PURE__*/ _jsx(DialogContent, {
                children: /*#__PURE__*/ _jsxs(DialogHeader, {
                    className: "nextjs-container-errors-header",
                    children: [
                        /*#__PURE__*/ _jsx("h3", {
                            id: "nextjs__container_errors_label",
                            children: "Missing required html tags"
                        }),
                        versionInfo ? /*#__PURE__*/ _jsx(VersionStalenessInfo, {
                            ...versionInfo
                        }) : null,
                        /*#__PURE__*/ _jsx("p", {
                            id: "nextjs__container_errors_desc",
                            className: "nextjs__container_errors_desc",
                            children: /*#__PURE__*/ _jsx(HotlinkedText, {
                                text: "The following tags are missing in the Root Layout: " + missingTags.map((tagName)=>"<" + tagName + ">").join(", ") + ".\nRead more at https://nextjs.org/docs/messages/missing-root-layout-tags"
                            })
                        })
                    ]
                })
            })
        })
    });
};

//# sourceMappingURL=root-layout-missing-tags-error.js.map