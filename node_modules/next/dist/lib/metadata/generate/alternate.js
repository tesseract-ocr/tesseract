"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "AlternatesMetadata", {
    enumerable: true,
    get: function() {
        return AlternatesMetadata;
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
function AlternateLink({ descriptor, ...props }) {
    if (!descriptor.url) return null;
    return /*#__PURE__*/ (0, _jsxruntime.jsx)("link", {
        ...props,
        ...descriptor.title && {
            title: descriptor.title
        },
        href: descriptor.url.toString()
    });
}
function AlternatesMetadata({ alternates }) {
    if (!alternates) return null;
    const { canonical, languages, media, types } = alternates;
    return (0, _meta.MetaFilter)([
        canonical ? AlternateLink({
            rel: 'canonical',
            descriptor: canonical
        }) : null,
        languages ? Object.entries(languages).flatMap(([locale, descriptors])=>descriptors == null ? void 0 : descriptors.map((descriptor)=>AlternateLink({
                    rel: 'alternate',
                    hrefLang: locale,
                    descriptor
                }))) : null,
        media ? Object.entries(media).flatMap(([mediaName, descriptors])=>descriptors == null ? void 0 : descriptors.map((descriptor)=>AlternateLink({
                    rel: 'alternate',
                    media: mediaName,
                    descriptor
                }))) : null,
        types ? Object.entries(types).flatMap(([type, descriptors])=>descriptors == null ? void 0 : descriptors.map((descriptor)=>AlternateLink({
                    rel: 'alternate',
                    type,
                    descriptor
                }))) : null
    ]);
}

//# sourceMappingURL=alternate.js.map