import { CACHE_ONE_YEAR } from '../../lib/constants';
export function formatRevalidate({ revalidate, expireTime }) {
    const swrHeader = typeof revalidate === 'number' && expireTime !== undefined ? revalidate >= expireTime ? '' : `stale-while-revalidate=${expireTime - revalidate}` : 'stale-while-revalidate';
    if (revalidate === 0) {
        return 'private, no-cache, no-store, max-age=0, must-revalidate';
    } else if (typeof revalidate === 'number') {
        return `s-maxage=${revalidate}, ${swrHeader}`;
    }
    return `s-maxage=${CACHE_ONE_YEAR}, ${swrHeader}`;
}

//# sourceMappingURL=revalidate.js.map