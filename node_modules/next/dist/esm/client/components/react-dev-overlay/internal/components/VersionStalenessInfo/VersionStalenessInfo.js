import { jsx as _jsx, jsxs as _jsxs } from "react/jsx-runtime";
import React from "react";
export function VersionStalenessInfo(props) {
    if (!props) return null;
    const { staleness } = props;
    let { text, indicatorClass, title } = getStaleness(props);
    if (!text) return null;
    return /*#__PURE__*/ _jsxs("small", {
        className: "nextjs-container-build-error-version-status",
        children: [
            /*#__PURE__*/ _jsx("span", {
                className: indicatorClass
            }),
            /*#__PURE__*/ _jsx("small", {
                "data-nextjs-version-checker": true,
                title: title,
                children: text
            }),
            " ",
            staleness === "fresh" || staleness === "newer-than-npm" || staleness === "unknown" ? null : /*#__PURE__*/ _jsx("a", {
                target: "_blank",
                rel: "noopener noreferrer",
                href: "https://nextjs.org/docs/messages/version-staleness",
                children: "(learn more)"
            }),
            process.env.TURBOPACK ? " (turbo)" : ""
        ]
    });
}
export function getStaleness(param) {
    let { installed, staleness, expected } = param;
    let text = "";
    let title = "";
    let indicatorClass = "";
    const versionLabel = "Next.js (" + installed + ")";
    switch(staleness){
        case "newer-than-npm":
        case "fresh":
            text = versionLabel;
            title = "Latest available version is detected (" + installed + ").";
            indicatorClass = "fresh";
            break;
        case "stale-patch":
        case "stale-minor":
            text = "" + versionLabel + " out of date";
            title = "There is a newer version (" + expected + ") available, upgrade recommended! ";
            indicatorClass = "stale";
            break;
        case "stale-major":
            {
                text = "" + versionLabel + " is outdated";
                title = "An outdated version detected (latest is " + expected + "), upgrade is highly recommended!";
                indicatorClass = "outdated";
                break;
            }
        case "stale-prerelease":
            {
                text = "" + versionLabel + " is outdated";
                title = "There is a newer canary version (" + expected + ") available, please upgrade! ";
                indicatorClass = "stale";
                break;
            }
        case "unknown":
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

//# sourceMappingURL=VersionStalenessInfo.js.map