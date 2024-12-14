import { NEXT_RSC_UNION_QUERY } from '../client/components/app-router-headers';
const DUMMY_ORIGIN = 'http://n';
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
export function stripNextRscUnionQuery(relativeUrl) {
    const urlInstance = new URL(relativeUrl, DUMMY_ORIGIN);
    urlInstance.searchParams.delete(NEXT_RSC_UNION_QUERY);
    return urlInstance.pathname + urlInstance.search;
}

//# sourceMappingURL=url.js.map