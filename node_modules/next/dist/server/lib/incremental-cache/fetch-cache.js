"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return FetchCache;
    }
});
const _responsecache = require("../../response-cache");
const _lrucache = require("../lru-cache");
const _constants = require("../../../lib/constants");
let rateLimitedUntil = 0;
let memoryCache;
const CACHE_TAGS_HEADER = 'x-vercel-cache-tags';
const CACHE_HEADERS_HEADER = 'x-vercel-sc-headers';
const CACHE_STATE_HEADER = 'x-vercel-cache-state';
const CACHE_REVALIDATE_HEADER = 'x-vercel-revalidate';
const CACHE_FETCH_URL_HEADER = 'x-vercel-cache-item-name';
const CACHE_CONTROL_VALUE_HEADER = 'x-vercel-cache-control';
const DEBUG = Boolean(process.env.NEXT_PRIVATE_DEBUG_CACHE);
async function fetchRetryWithTimeout(url, init, retryIndex = 0) {
    const controller = new AbortController();
    const timeout = setTimeout(()=>{
        controller.abort();
    }, 500);
    return fetch(url, {
        ...init || {},
        signal: controller.signal
    }).catch((err)=>{
        if (retryIndex === 3) {
            throw err;
        } else {
            if (DEBUG) {
                console.log(`Fetch failed for ${url} retry ${retryIndex}`);
            }
            return fetchRetryWithTimeout(url, init, retryIndex + 1);
        }
    }).finally(()=>{
        clearTimeout(timeout);
    });
}
class FetchCache {
    hasMatchingTags(arr1, arr2) {
        if (arr1.length !== arr2.length) return false;
        const set1 = new Set(arr1);
        const set2 = new Set(arr2);
        if (set1.size !== set2.size) return false;
        for (let tag of set1){
            if (!set2.has(tag)) return false;
        }
        return true;
    }
    static isAvailable(ctx) {
        return !!(ctx._requestHeaders['x-vercel-sc-host'] || process.env.SUSPENSE_CACHE_URL);
    }
    constructor(ctx){
        this.headers = {};
        this.headers['Content-Type'] = 'application/json';
        if (CACHE_HEADERS_HEADER in ctx._requestHeaders) {
            const newHeaders = JSON.parse(ctx._requestHeaders[CACHE_HEADERS_HEADER]);
            for(const k in newHeaders){
                this.headers[k] = newHeaders[k];
            }
            delete ctx._requestHeaders[CACHE_HEADERS_HEADER];
        }
        const scHost = ctx._requestHeaders['x-vercel-sc-host'] || process.env.SUSPENSE_CACHE_URL;
        const scBasePath = ctx._requestHeaders['x-vercel-sc-basepath'] || process.env.SUSPENSE_CACHE_BASEPATH;
        if (process.env.SUSPENSE_CACHE_AUTH_TOKEN) {
            this.headers['Authorization'] = `Bearer ${process.env.SUSPENSE_CACHE_AUTH_TOKEN}`;
        }
        if (scHost) {
            const scProto = process.env.SUSPENSE_CACHE_PROTO || 'https';
            this.cacheEndpoint = `${scProto}://${scHost}${scBasePath || ''}`;
            if (DEBUG) {
                console.log('using cache endpoint', this.cacheEndpoint);
            }
        } else if (DEBUG) {
            console.log('no cache endpoint available');
        }
        if (ctx.maxMemoryCacheSize) {
            if (!memoryCache) {
                if (DEBUG) {
                    console.log('using memory store for fetch cache');
                }
                memoryCache = new _lrucache.LRUCache(ctx.maxMemoryCacheSize, function length({ value }) {
                    var _JSON_stringify;
                    if (!value) {
                        return 25;
                    } else if (value.kind === _responsecache.CachedRouteKind.REDIRECT) {
                        return JSON.stringify(value.props).length;
                    } else if (value.kind === _responsecache.CachedRouteKind.IMAGE) {
                        throw new Error('invariant image should not be incremental-cache');
                    } else if (value.kind === _responsecache.CachedRouteKind.FETCH) {
                        return JSON.stringify(value.data || '').length;
                    } else if (value.kind === _responsecache.CachedRouteKind.APP_ROUTE) {
                        return value.body.length;
                    }
                    // rough estimate of size of cache value
                    return value.html.length + (((_JSON_stringify = JSON.stringify(value.kind === _responsecache.CachedRouteKind.APP_PAGE ? value.rscData : value.pageData)) == null ? void 0 : _JSON_stringify.length) || 0);
                });
            }
        } else {
            if (DEBUG) {
                console.log('not using memory store for fetch cache');
            }
        }
    }
    resetRequestCache() {
        memoryCache == null ? void 0 : memoryCache.reset();
    }
    async revalidateTag(...args) {
        let [tags] = args;
        tags = typeof tags === 'string' ? [
            tags
        ] : tags;
        if (DEBUG) {
            console.log('revalidateTag', tags);
        }
        if (!tags.length) return;
        if (Date.now() < rateLimitedUntil) {
            if (DEBUG) {
                console.log('rate limited ', rateLimitedUntil);
            }
            return;
        }
        for(let i = 0; i < Math.ceil(tags.length / 64); i++){
            const currentTags = tags.slice(i * 64, i * 64 + 64);
            try {
                const res = await fetchRetryWithTimeout(`${this.cacheEndpoint}/v1/suspense-cache/revalidate?tags=${currentTags.map((tag)=>encodeURIComponent(tag)).join(',')}`, {
                    method: 'POST',
                    headers: this.headers,
                    // @ts-expect-error not on public type
                    next: {
                        internal: true
                    }
                });
                if (res.status === 429) {
                    const retryAfter = res.headers.get('retry-after') || '60000';
                    rateLimitedUntil = Date.now() + parseInt(retryAfter);
                }
                if (!res.ok) {
                    throw new Error(`Request failed with status ${res.status}.`);
                }
            } catch (err) {
                console.warn(`Failed to revalidate tag`, currentTags, err);
            }
        }
    }
    async get(...args) {
        var _data_value;
        const [key, ctx] = args;
        const { tags, softTags, kind: kindHint, fetchIdx, fetchUrl } = ctx;
        if (kindHint !== _responsecache.IncrementalCacheKind.FETCH) {
            return null;
        }
        if (Date.now() < rateLimitedUntil) {
            if (DEBUG) {
                console.log('rate limited');
            }
            return null;
        }
        // memory cache is cleared at the end of each request
        // so that revalidate events are pulled from upstream
        // on successive requests
        let data = memoryCache == null ? void 0 : memoryCache.get(key);
        const hasFetchKindAndMatchingTags = (data == null ? void 0 : (_data_value = data.value) == null ? void 0 : _data_value.kind) === _responsecache.CachedRouteKind.FETCH && this.hasMatchingTags(tags ?? [], data.value.tags ?? []);
        // Get data from fetch cache. Also check if new tags have been
        // specified with the same cache key (fetch URL)
        if (this.cacheEndpoint && (!data || !hasFetchKindAndMatchingTags)) {
            try {
                const start = Date.now();
                const fetchParams = {
                    internal: true,
                    fetchType: 'cache-get',
                    fetchUrl: fetchUrl,
                    fetchIdx
                };
                const res = await fetch(`${this.cacheEndpoint}/v1/suspense-cache/${key}`, {
                    method: 'GET',
                    headers: {
                        ...this.headers,
                        [CACHE_FETCH_URL_HEADER]: fetchUrl,
                        [CACHE_TAGS_HEADER]: (tags == null ? void 0 : tags.join(',')) || '',
                        [_constants.NEXT_CACHE_SOFT_TAGS_HEADER]: (softTags == null ? void 0 : softTags.join(',')) || ''
                    },
                    next: fetchParams
                });
                if (res.status === 429) {
                    const retryAfter = res.headers.get('retry-after') || '60000';
                    rateLimitedUntil = Date.now() + parseInt(retryAfter);
                }
                if (res.status === 404) {
                    if (DEBUG) {
                        console.log(`no fetch cache entry for ${key}, duration: ${Date.now() - start}ms`);
                    }
                    return null;
                }
                if (!res.ok) {
                    console.error(await res.text());
                    throw new Error(`invalid response from cache ${res.status}`);
                }
                const cached = await res.json();
                if (!cached || cached.kind !== _responsecache.CachedRouteKind.FETCH) {
                    DEBUG && console.log({
                        cached
                    });
                    throw new Error('invalid cache value');
                }
                // if new tags were specified, merge those tags to the existing tags
                if (cached.kind === _responsecache.CachedRouteKind.FETCH) {
                    cached.tags ??= [];
                    for (const tag of tags ?? []){
                        if (!cached.tags.includes(tag)) {
                            cached.tags.push(tag);
                        }
                    }
                }
                const cacheState = res.headers.get(CACHE_STATE_HEADER);
                const age = res.headers.get('age');
                data = {
                    value: cached,
                    // if it's already stale set it to a time in the past
                    // if not derive last modified from age
                    lastModified: cacheState !== 'fresh' ? Date.now() - _constants.CACHE_ONE_YEAR : Date.now() - parseInt(age || '0', 10) * 1000
                };
                if (DEBUG) {
                    console.log(`got fetch cache entry for ${key}, duration: ${Date.now() - start}ms, size: ${Object.keys(cached).length}, cache-state: ${cacheState} tags: ${tags == null ? void 0 : tags.join(',')} softTags: ${softTags == null ? void 0 : softTags.join(',')}`);
                }
                if (data) {
                    memoryCache == null ? void 0 : memoryCache.set(key, data);
                }
            } catch (err) {
                // unable to get data from fetch-cache
                if (DEBUG) {
                    console.error(`Failed to get from fetch-cache`, err);
                }
            }
        }
        return data || null;
    }
    async set(...args) {
        const [key, data, ctx] = args;
        const { fetchCache, fetchIdx, fetchUrl, tags } = ctx;
        if (!fetchCache) return;
        if (Date.now() < rateLimitedUntil) {
            if (DEBUG) {
                console.log('rate limited');
            }
            return;
        }
        memoryCache == null ? void 0 : memoryCache.set(key, {
            value: data,
            lastModified: Date.now()
        });
        if (this.cacheEndpoint) {
            try {
                const start = Date.now();
                if (data !== null && 'revalidate' in data) {
                    this.headers[CACHE_REVALIDATE_HEADER] = data.revalidate.toString();
                }
                if (!this.headers[CACHE_REVALIDATE_HEADER] && data !== null && 'data' in data) {
                    this.headers[CACHE_CONTROL_VALUE_HEADER] = data.data.headers['cache-control'];
                }
                const body = JSON.stringify({
                    ...data,
                    // we send the tags in the header instead
                    // of in the body here
                    tags: undefined
                });
                if (DEBUG) {
                    console.log('set cache', key);
                }
                const fetchParams = {
                    internal: true,
                    fetchType: 'cache-set',
                    fetchUrl,
                    fetchIdx
                };
                const res = await fetch(`${this.cacheEndpoint}/v1/suspense-cache/${key}`, {
                    method: 'POST',
                    headers: {
                        ...this.headers,
                        [CACHE_FETCH_URL_HEADER]: fetchUrl || '',
                        [CACHE_TAGS_HEADER]: (tags == null ? void 0 : tags.join(',')) || ''
                    },
                    body: body,
                    next: fetchParams
                });
                if (res.status === 429) {
                    const retryAfter = res.headers.get('retry-after') || '60000';
                    rateLimitedUntil = Date.now() + parseInt(retryAfter);
                }
                if (!res.ok) {
                    DEBUG && console.log(await res.text());
                    throw new Error(`invalid response ${res.status}`);
                }
                if (DEBUG) {
                    console.log(`successfully set to fetch-cache for ${key}, duration: ${Date.now() - start}ms, size: ${body.length}`);
                }
            } catch (err) {
                // unable to set to fetch-cache
                if (DEBUG) {
                    console.error(`Failed to update fetch cache`, err);
                }
            }
        }
        return;
    }
}

//# sourceMappingURL=fetch-cache.js.map