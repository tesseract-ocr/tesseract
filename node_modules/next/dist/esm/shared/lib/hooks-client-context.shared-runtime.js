"use client";

import { createContext } from "react";
export const SearchParamsContext = createContext(null);
export const PathnameContext = createContext(null);
export const PathParamsContext = createContext(null);
if (process.env.NODE_ENV !== "production") {
    SearchParamsContext.displayName = "SearchParamsContext";
    PathnameContext.displayName = "PathnameContext";
    PathParamsContext.displayName = "PathParamsContext";
}

//# sourceMappingURL=hooks-client-context.shared-runtime.js.map