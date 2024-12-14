import { jsx as _jsx } from "react/jsx-runtime";
import { HTTPAccessErrorFallback } from './http-access-fallback/error-fallback';
export default function NotFound() {
    return /*#__PURE__*/ _jsx(HTTPAccessErrorFallback, {
        status: 404,
        message: "This page could not be found."
    });
}

//# sourceMappingURL=not-found-error.js.map