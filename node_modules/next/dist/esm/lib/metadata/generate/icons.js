import { jsx as _jsx } from "react/jsx-runtime";
import React from "react";
import { MetaFilter } from "./meta";
function IconDescriptorLink({ icon }) {
    const { url, rel = "icon", ...props } = icon;
    return /*#__PURE__*/ _jsx("link", {
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
        return /*#__PURE__*/ _jsx("link", {
            rel: rel,
            href: href
        });
    }
}
export function IconsMetadata({ icons }) {
    if (!icons) return null;
    const shortcutList = icons.shortcut;
    const iconList = icons.icon;
    const appleList = icons.apple;
    const otherList = icons.other;
    return MetaFilter([
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