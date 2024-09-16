"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    AppleWebAppMeta: null,
    BasicMeta: null,
    FacebookMeta: null,
    FormatDetectionMeta: null,
    ItunesMeta: null,
    VerificationMeta: null,
    ViewportMeta: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    AppleWebAppMeta: function() {
        return AppleWebAppMeta;
    },
    BasicMeta: function() {
        return BasicMeta;
    },
    FacebookMeta: function() {
        return FacebookMeta;
    },
    FormatDetectionMeta: function() {
        return FormatDetectionMeta;
    },
    ItunesMeta: function() {
        return ItunesMeta;
    },
    VerificationMeta: function() {
        return VerificationMeta;
    },
    ViewportMeta: function() {
        return ViewportMeta;
    }
});
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_default(require("react"));
const _meta = require("./meta");
const _constants = require("../constants");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
// convert viewport object to string for viewport meta tag
function resolveViewportLayout(viewport) {
    let resolved = null;
    if (viewport && typeof viewport === "object") {
        resolved = "";
        for(const viewportKey_ in _constants.ViewportMetaKeys){
            const viewportKey = viewportKey_;
            if (viewportKey in viewport) {
                let value = viewport[viewportKey];
                if (typeof value === "boolean") value = value ? "yes" : "no";
                if (resolved) resolved += ", ";
                resolved += `${_constants.ViewportMetaKeys[viewportKey]}=${value}`;
            }
        }
    }
    return resolved;
}
function ViewportMeta({ viewport }) {
    return (0, _meta.MetaFilter)([
        (0, _meta.Meta)({
            name: "viewport",
            content: resolveViewportLayout(viewport)
        }),
        ...viewport.themeColor ? viewport.themeColor.map((themeColor)=>(0, _meta.Meta)({
                name: "theme-color",
                content: themeColor.color,
                media: themeColor.media
            })) : [],
        (0, _meta.Meta)({
            name: "color-scheme",
            content: viewport.colorScheme
        })
    ]);
}
function BasicMeta({ metadata }) {
    var _metadata_keywords, _metadata_robots, _metadata_robots1;
    return (0, _meta.MetaFilter)([
        /*#__PURE__*/ (0, _jsxruntime.jsx)("meta", {
            charSet: "utf-8"
        }),
        metadata.title !== null && metadata.title.absolute ? /*#__PURE__*/ (0, _jsxruntime.jsx)("title", {
            children: metadata.title.absolute
        }) : null,
        (0, _meta.Meta)({
            name: "description",
            content: metadata.description
        }),
        (0, _meta.Meta)({
            name: "application-name",
            content: metadata.applicationName
        }),
        ...metadata.authors ? metadata.authors.map((author)=>[
                author.url ? /*#__PURE__*/ (0, _jsxruntime.jsx)("link", {
                    rel: "author",
                    href: author.url.toString()
                }) : null,
                (0, _meta.Meta)({
                    name: "author",
                    content: author.name
                })
            ]) : [],
        metadata.manifest ? /*#__PURE__*/ (0, _jsxruntime.jsx)("link", {
            rel: "manifest",
            href: metadata.manifest.toString(),
            crossOrigin: "use-credentials"
        }) : null,
        (0, _meta.Meta)({
            name: "generator",
            content: metadata.generator
        }),
        (0, _meta.Meta)({
            name: "keywords",
            content: (_metadata_keywords = metadata.keywords) == null ? void 0 : _metadata_keywords.join(",")
        }),
        (0, _meta.Meta)({
            name: "referrer",
            content: metadata.referrer
        }),
        (0, _meta.Meta)({
            name: "creator",
            content: metadata.creator
        }),
        (0, _meta.Meta)({
            name: "publisher",
            content: metadata.publisher
        }),
        (0, _meta.Meta)({
            name: "robots",
            content: (_metadata_robots = metadata.robots) == null ? void 0 : _metadata_robots.basic
        }),
        (0, _meta.Meta)({
            name: "googlebot",
            content: (_metadata_robots1 = metadata.robots) == null ? void 0 : _metadata_robots1.googleBot
        }),
        (0, _meta.Meta)({
            name: "abstract",
            content: metadata.abstract
        }),
        ...metadata.archives ? metadata.archives.map((archive)=>/*#__PURE__*/ (0, _jsxruntime.jsx)("link", {
                rel: "archives",
                href: archive
            })) : [],
        ...metadata.assets ? metadata.assets.map((asset)=>/*#__PURE__*/ (0, _jsxruntime.jsx)("link", {
                rel: "assets",
                href: asset
            })) : [],
        ...metadata.bookmarks ? metadata.bookmarks.map((bookmark)=>/*#__PURE__*/ (0, _jsxruntime.jsx)("link", {
                rel: "bookmarks",
                href: bookmark
            })) : [],
        (0, _meta.Meta)({
            name: "category",
            content: metadata.category
        }),
        (0, _meta.Meta)({
            name: "classification",
            content: metadata.classification
        }),
        ...metadata.other ? Object.entries(metadata.other).map(([name, content])=>{
            if (Array.isArray(content)) {
                return content.map((contentItem)=>(0, _meta.Meta)({
                        name,
                        content: contentItem
                    }));
            } else {
                return (0, _meta.Meta)({
                    name,
                    content
                });
            }
        }) : []
    ]);
}
function ItunesMeta({ itunes }) {
    if (!itunes) return null;
    const { appId, appArgument } = itunes;
    let content = `app-id=${appId}`;
    if (appArgument) {
        content += `, app-argument=${appArgument}`;
    }
    return /*#__PURE__*/ (0, _jsxruntime.jsx)("meta", {
        name: "apple-itunes-app",
        content: content
    });
}
function FacebookMeta({ facebook }) {
    if (!facebook) return null;
    const { appId, admins } = facebook;
    return (0, _meta.MetaFilter)([
        appId ? /*#__PURE__*/ (0, _jsxruntime.jsx)("meta", {
            property: "fb:app_id",
            content: appId
        }) : null,
        ...admins ? admins.map((admin)=>/*#__PURE__*/ (0, _jsxruntime.jsx)("meta", {
                property: "fb:admins",
                content: admin
            })) : []
    ]);
}
const formatDetectionKeys = [
    "telephone",
    "date",
    "address",
    "email",
    "url"
];
function FormatDetectionMeta({ formatDetection }) {
    if (!formatDetection) return null;
    let content = "";
    for (const key of formatDetectionKeys){
        if (key in formatDetection) {
            if (content) content += ", ";
            content += `${key}=no`;
        }
    }
    return /*#__PURE__*/ (0, _jsxruntime.jsx)("meta", {
        name: "format-detection",
        content: content
    });
}
function AppleWebAppMeta({ appleWebApp }) {
    if (!appleWebApp) return null;
    const { capable, title, startupImage, statusBarStyle } = appleWebApp;
    return (0, _meta.MetaFilter)([
        capable ? (0, _meta.Meta)({
            name: "apple-mobile-web-app-capable",
            content: "yes"
        }) : null,
        (0, _meta.Meta)({
            name: "apple-mobile-web-app-title",
            content: title
        }),
        startupImage ? startupImage.map((image)=>/*#__PURE__*/ (0, _jsxruntime.jsx)("link", {
                href: image.url,
                media: image.media,
                rel: "apple-touch-startup-image"
            })) : null,
        statusBarStyle ? (0, _meta.Meta)({
            name: "apple-mobile-web-app-status-bar-style",
            content: statusBarStyle
        }) : null
    ]);
}
function VerificationMeta({ verification }) {
    if (!verification) return null;
    return (0, _meta.MetaFilter)([
        (0, _meta.MultiMeta)({
            namePrefix: "google-site-verification",
            contents: verification.google
        }),
        (0, _meta.MultiMeta)({
            namePrefix: "y_key",
            contents: verification.yahoo
        }),
        (0, _meta.MultiMeta)({
            namePrefix: "yandex-verification",
            contents: verification.yandex
        }),
        (0, _meta.MultiMeta)({
            namePrefix: "me",
            contents: verification.me
        }),
        ...verification.other ? Object.entries(verification.other).map(([key, value])=>(0, _meta.MultiMeta)({
                namePrefix: key,
                contents: value
            })) : []
    ]);
}

//# sourceMappingURL=basic.js.map