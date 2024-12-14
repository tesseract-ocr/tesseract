export const ACTION_REFRESH = 'refresh';
export const ACTION_NAVIGATE = 'navigate';
export const ACTION_RESTORE = 'restore';
export const ACTION_SERVER_PATCH = 'server-patch';
export const ACTION_PREFETCH = 'prefetch';
export const ACTION_HMR_REFRESH = 'hmr-refresh';
export const ACTION_SERVER_ACTION = 'server-action';
/**
 * PrefetchKind defines the type of prefetching that should be done.
 * - `auto` - if the page is dynamic, prefetch the page data partially, if static prefetch the page data fully.
 * - `full` - prefetch the page data fully.
 * - `temporary` - a temporary prefetch entry is added to the cache, this is used when prefetch={false} is used in next/link or when you push a route programmatically.
 */ export var PrefetchKind = /*#__PURE__*/ function(PrefetchKind) {
    PrefetchKind["AUTO"] = "auto";
    PrefetchKind["FULL"] = "full";
    PrefetchKind["TEMPORARY"] = "temporary";
    return PrefetchKind;
}({});
export var PrefetchCacheEntryStatus = /*#__PURE__*/ function(PrefetchCacheEntryStatus) {
    PrefetchCacheEntryStatus["fresh"] = "fresh";
    PrefetchCacheEntryStatus["reusable"] = "reusable";
    PrefetchCacheEntryStatus["expired"] = "expired";
    PrefetchCacheEntryStatus["stale"] = "stale";
    return PrefetchCacheEntryStatus;
}({});

//# sourceMappingURL=router-reducer-types.js.map