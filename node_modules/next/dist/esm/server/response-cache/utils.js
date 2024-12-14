import { CachedRouteKind, IncrementalCacheKind } from './types';
import RenderResult from '../render-result';
import { RouteKind } from '../route-kind';
export async function fromResponseCacheEntry(cacheEntry) {
    var _cacheEntry_value, _cacheEntry_value1;
    return {
        ...cacheEntry,
        value: ((_cacheEntry_value = cacheEntry.value) == null ? void 0 : _cacheEntry_value.kind) === CachedRouteKind.PAGES ? {
            kind: CachedRouteKind.PAGES,
            html: await cacheEntry.value.html.toUnchunkedString(true),
            pageData: cacheEntry.value.pageData,
            headers: cacheEntry.value.headers,
            status: cacheEntry.value.status
        } : ((_cacheEntry_value1 = cacheEntry.value) == null ? void 0 : _cacheEntry_value1.kind) === CachedRouteKind.APP_PAGE ? {
            kind: CachedRouteKind.APP_PAGE,
            html: await cacheEntry.value.html.toUnchunkedString(true),
            postponed: cacheEntry.value.postponed,
            rscData: cacheEntry.value.rscData,
            headers: cacheEntry.value.headers,
            status: cacheEntry.value.status,
            segmentData: cacheEntry.value.segmentData
        } : cacheEntry.value
    };
}
export async function toResponseCacheEntry(response) {
    var _response_value, _response_value1, _response_value2;
    if (!response) return null;
    if (((_response_value = response.value) == null ? void 0 : _response_value.kind) === CachedRouteKind.FETCH) {
        throw new Error('Invariant: unexpected cachedResponse of kind fetch in response cache');
    }
    return {
        isMiss: response.isMiss,
        isStale: response.isStale,
        revalidate: response.revalidate,
        isFallback: response.isFallback,
        value: ((_response_value1 = response.value) == null ? void 0 : _response_value1.kind) === CachedRouteKind.PAGES ? {
            kind: CachedRouteKind.PAGES,
            html: RenderResult.fromStatic(response.value.html),
            pageData: response.value.pageData,
            headers: response.value.headers,
            status: response.value.status
        } : ((_response_value2 = response.value) == null ? void 0 : _response_value2.kind) === CachedRouteKind.APP_PAGE ? {
            kind: CachedRouteKind.APP_PAGE,
            html: RenderResult.fromStatic(response.value.html),
            rscData: response.value.rscData,
            headers: response.value.headers,
            status: response.value.status,
            postponed: response.value.postponed,
            segmentData: response.value.segmentData
        } : response.value
    };
}
export function routeKindToIncrementalCacheKind(routeKind) {
    switch(routeKind){
        case RouteKind.PAGES:
            return IncrementalCacheKind.PAGES;
        case RouteKind.APP_PAGE:
            return IncrementalCacheKind.APP_PAGE;
        case RouteKind.IMAGE:
            return IncrementalCacheKind.IMAGE;
        case RouteKind.APP_ROUTE:
            return IncrementalCacheKind.APP_ROUTE;
        default:
            throw new Error(`Unexpected route kind ${routeKind}`);
    }
}

//# sourceMappingURL=utils.js.map