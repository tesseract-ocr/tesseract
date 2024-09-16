"use client";

import { jsx as _jsx, Fragment as _Fragment } from "react/jsx-runtime";
import React, { useContext } from "react";
import { TemplateContext } from "../../shared/lib/app-router-context.shared-runtime";
export default function RenderFromTemplateContext() {
    const children = useContext(TemplateContext);
    return /*#__PURE__*/ _jsx(_Fragment, {
        children: children
    });
}

//# sourceMappingURL=render-from-template-context.js.map