"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "IconsMetadata", {
    enumerable: true,
    get: function() {
        return IconsMetadata;
    }
});
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_default(require("react"));
const _meta = require("./meta");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function IconDescriptorLink({ icon }) {
    const { url, rel = "icon", ...props } = icon;
    return /*#__PURE__*/ (0, _jsxruntime.jsx)("link", {
        rel: rel,
        href: url.toString(),
        ...props
    });
}
function IconLink({ rel, icon }) {
    if (typeof icon === "object" && !(icon instanceof URL)) {
        if (!icon.rel && rel) icon.rel = rel;
        return IconDescriptorLink({
            icon
        });
    } else {
        const href = icon.toString();
        return /*#__PURE__*/ (0, _jsxruntime.jsx)("link", {
            rel: rel,
            href: href
        });
    }
}
function IconsMetadata({ icons }) {
    if (!icons) return null;
    const shortcutList = icons.shortcut;
    const iconList = icons.icon;
    const appleList = icons.apple;
    const otherList = icons.other;
    return (0, _meta.MetaFilter)([
        shortcutList ? shortcutList.map((icon)=>IconLink({
                rel: "shortcut icon",
                icon
            })) : null,
        iconList ? iconList.map((icon)=>IconLink({
                rel: "icon",
                icon
            })) : null,
        appleList ? appleList.map((icon)=>IconLink({
                rel: "apple-touch-icon",
                icon
            })) : null,
        otherList ? otherList.map((icon)=>IconDescriptorLink({
                icon
            })) : null
    ]);
}

//# sourceMappingURL=icons.js.map