/*

Files in the rsc directory are meant to be packaged as part of the RSC graph using next-app-loader.

*/ import ReactDOM from "react-dom";
export function preloadStyle(href, crossOrigin) {
    const opts = {
        as: "style"
    };
    if (typeof crossOrigin === "string") {
        opts.crossOrigin = crossOrigin;
    }
    ReactDOM.preload(href, opts);
}
export function preloadFont(href, type, crossOrigin) {
    const opts = {
        as: "font",
        type
    };
    if (typeof crossOrigin === "string") {
        opts.crossOrigin = crossOrigin;
    }
    ReactDOM.preload(href, opts);
}
export function preconnect(href, crossOrigin) {
    ReactDOM.preconnect(href, typeof crossOrigin === "string" ? {
        crossOrigin
    } : undefined);
}

//# sourceMappingURL=preloads.js.map