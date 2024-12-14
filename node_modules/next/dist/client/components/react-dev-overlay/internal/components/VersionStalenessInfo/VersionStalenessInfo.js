"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    VersionStalenessInfo: null,
    getStaleness: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    VersionStalenessInfo: function() {
        return VersionStalenessInfo;
    },
    getStaleness: function() {
        return getStaleness;
    }
});
const _jsxruntime = require("react/jsx-runtime");
function VersionStalenessInfo(param) {
    let { versionInfo } = param;
    if (!versionInfo) return null;
    const { staleness } = versionInfo;
    let { text, indicatorClass, title } = getStaleness(versionInfo);
    if (!text) return null;
    return /*#__PURE__*/ (0, _jsxruntime.jsxs)("span", {
        className: "nextjs-container-build-error-version-status",
        children: [
            /*#__PURE__*/ (0, _jsxruntime.jsx)("span", {
                className: indicatorClass
            }),
            /*#__PURE__*/ (0, _jsxruntime.jsx)("small", {
                "data-nextjs-version-checker": true,
                title: title,
                children: text
            }),
            ' ',
            staleness === 'fresh' || staleness === 'newer-than-npm' || staleness === 'unknown' ? null : /*#__PURE__*/ (0, _jsxruntime.jsx)("a", {
                target: "_blank",
                rel: "noopener noreferrer",
                href: "https://nextjs.org/docs/messages/version-staleness",
                children: "(learn more)"
            }),
            process.env.TURBOPACK ? ' (Turbopack)' : ''
        ]
    });
}
function getStaleness(param) {
    let { installed, staleness, expected } = param;
    let text = '';
    let title = '';
    let indicatorClass = '';
    const versionLabel = "Next.js (" + installed + ")";
    switch(staleness){
        case 'newer-than-npm':
        case 'fresh':
            text = versionLabel;
            title = "Latest available version is detected (" + installed + ").";
            indicatorClass = 'fresh';
            break;
        case 'stale-patch':
        case 'stale-minor':
            text = "" + versionLabel + " out of date";
            title = "There is a newer version (" + expected + ") available, upgrade recommended! ";
            indicatorClass = 'stale';
            break;
        case 'stale-major':
            {
                text = "" + versionLabel + " is outdated";
                title = "An outdated version detected (latest is " + expected + "), upgrade is highly recommended!";
                indicatorClass = 'outdated';
                break;
            }
        case 'stale-prerelease':
            {
                text = "" + versionLabel + " is outdated";
                title = "There is a newer canary version (" + expected + ") available, please upgrade! ";
                indicatorClass = 'stale';
                break;
            }
        case 'unknown':
            break;
        default:
            break;
    }
    return {
        text,
        indicatorClass,
        title
    };
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=VersionStalenessInfo.js.map