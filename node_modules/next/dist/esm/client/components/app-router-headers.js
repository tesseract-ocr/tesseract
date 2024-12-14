export const RSC_HEADER = 'RSC';
export const ACTION_HEADER = 'Next-Action';
// TODO: Instead of sending the full router state, we only need to send the
// segment path. Saves bytes. Then we could also use this field for segment
// prefetches, which also need to specify a particular segment.
export const NEXT_ROUTER_STATE_TREE_HEADER = 'Next-Router-State-Tree';
export const NEXT_ROUTER_PREFETCH_HEADER = 'Next-Router-Prefetch';
// This contains the path to the segment being prefetched.
// TODO: If we change Next-Router-State-Tree to be a segment path, we can use
// that instead. Then Next-Router-Prefetch and Next-Router-Segment-Prefetch can
// be merged into a single enum.
export const NEXT_ROUTER_SEGMENT_PREFETCH_HEADER = 'Next-Router-Segment-Prefetch';
export const NEXT_HMR_REFRESH_HEADER = 'Next-HMR-Refresh';
export const NEXT_URL = 'Next-Url';
export const RSC_CONTENT_TYPE_HEADER = 'text/x-component';
export const FLIGHT_HEADERS = [
    RSC_HEADER,
    NEXT_ROUTER_STATE_TREE_HEADER,
    NEXT_ROUTER_PREFETCH_HEADER,
    NEXT_HMR_REFRESH_HEADER,
    NEXT_ROUTER_SEGMENT_PREFETCH_HEADER
];
export const NEXT_RSC_UNION_QUERY = '_rsc';
export const NEXT_ROUTER_STALE_TIME_HEADER = 'x-nextjs-stale-time';
export const NEXT_DID_POSTPONE_HEADER = 'x-nextjs-postponed';
export const NEXT_IS_PRERENDER_HEADER = 'x-nextjs-prerender';

//# sourceMappingURL=app-router-headers.js.map