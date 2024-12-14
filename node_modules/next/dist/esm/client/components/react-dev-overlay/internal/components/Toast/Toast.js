import { jsx as _jsx } from "react/jsx-runtime";
import * as React from 'react';
export const Toast = function Toast(param) {
    let { onClick, children, className, ...props } = param;
    return /*#__PURE__*/ _jsx("div", {
        ...props,
        onClick: (e)=>{
            e.preventDefault();
            return onClick == null ? void 0 : onClick();
        },
        className: "nextjs-toast" + (className ? ' ' + className : ''),
        children: /*#__PURE__*/ _jsx("div", {
            "data-nextjs-toast-wrapper": true,
            children: children
        })
    });
};

//# sourceMappingURL=Toast.js.map