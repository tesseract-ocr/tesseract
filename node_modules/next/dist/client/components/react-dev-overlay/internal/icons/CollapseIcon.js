"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "CollapseIcon", {
    enumerable: true,
    get: function() {
        return CollapseIcon;
    }
});
const _jsxruntime = require("react/jsx-runtime");
function CollapseIcon(param) {
    let { collapsed } = param === void 0 ? {} : param;
    return /*#__PURE__*/ (0, _jsxruntime.jsx)("svg", {
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
        children: /*#__PURE__*/ (0, _jsxruntime.jsx)("path", {
            d: "M9 18l6-6-6-6"
        })
    });
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=CollapseIcon.js.map