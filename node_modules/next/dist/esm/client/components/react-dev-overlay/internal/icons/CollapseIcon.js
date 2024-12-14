import { jsx as _jsx } from "react/jsx-runtime";
export function CollapseIcon(param) {
    let { collapsed } = param === void 0 ? {} : param;
    return /*#__PURE__*/ _jsx("svg", {
        "data-nextjs-call-stack-chevron-icon": true,
        "data-collapsed": collapsed,
        fill: "none",
        height: "20",
        width: "20",
        shapeRendering: "geometricPrecision",
        stroke: "currentColor",
        strokeLinecap: "round",
        strokeLinejoin: "round",
        strokeWidth: "2",
        viewBox: "0 0 24 24",
        ...typeof collapsed === 'boolean' ? {
            style: {
                transform: collapsed ? undefined : 'rotate(90deg)'
            }
        } : {},
        children: /*#__PURE__*/ _jsx("path", {
            d: "M9 18l6-6-6-6"
        })
    });
}

//# sourceMappingURL=CollapseIcon.js.map