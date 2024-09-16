const DUMMY_ORIGIN = "http://n";
function getUrlWithoutHost(url) {
    return new URL(url, DUMMY_ORIGIN);
}
export function getPathname(url) {
    return getUrlWithoutHost(url).pathname;
}
export function isFullStringUrl(url) {
    return /https?:\/\//.test(url);
}
export function parseUrl(url) {
    let parsed = undefined;
    try {
        parsed = new URL(url, DUMMY_ORIGIN);
    } catch  {}
    return parsed;
}

//# sourceMappingURL=url.js.map