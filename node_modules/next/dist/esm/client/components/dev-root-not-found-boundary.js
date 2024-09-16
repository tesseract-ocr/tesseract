"use client";

import { jsx as _jsx } from "react/jsx-runtime";
import React from "react";
import { NotFoundBoundary } from "./not-found-boundary";
export function bailOnNotFound() {
    throw new Error("notFound() is not allowed to use in root layout");
}
function NotAllowedRootNotFoundError() {
    bailOnNotFound();
    return null;
}
export function DevRootNotFoundBoundary(param) {
    let { children } = param;
    return /*#__PURE__*/ _jsx(NotFoundBoundary, {
        notFound: /*#__PURE__*/ _jsx(NotAllowedRootNotFoundError, {}),
        children: children
    });
}

//# sourceMappingURL=dev-root-not-found-boundary.js.map