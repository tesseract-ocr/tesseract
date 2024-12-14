"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return FileSystemCache;
    }
});
const _responsecache = require("../../response-cache");
const _lrucache = require("../lru-cache");
const _path = /*#__PURE__*/ _interop_require_default(require("../../../shared/lib/isomorphic/path"));
const _constants = require("../../../lib/constants");
const _tagsmanifestexternal = require("./tags-manifest.external");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
let memoryCache;
class FileSystemCache {
    constructor(ctx){
        this.fs = ctx.fs;
        this.flushToDisk = ctx.flushToDisk;
        this.serverDistDir = ctx.serverDistDir;
        this.revalidatedTags = ctx.revalidatedTags;
        this.debug = !!process.env.NEXT_PRIVATE_DEBUG_CACHE;
        if (ctx.maxMemoryCacheSize) {
            if (!memoryCache) {
                if (this.debug) {
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
        } else if (this.debug) {
            console.log('not using memory store for fetch cache');
        }
    }
    resetRequestCache() {}
    async revalidateTag(...args) {
        let [tags] = args;
        tags = typeof tags === 'string' ? [
            tags
        ] : tags;
        if (this.debug) {
            console.log('revalidateTag', tags);
        }
        if (tags.length === 0) {
            return;
        }
        for (const tag of tags){
            const data = _tagsmanifestexternal.tagsManifest.items[tag] || {};
            data.revalidatedAt = Date.now();
            _tagsmanifestexternal.tagsManifest.items[tag] = data;
        }
    }
    async get(...args) {
        var _data_value, _data_value1, _data_value2;
        const [key, ctx] = args;
        const { tags, softTags, kind, isRoutePPREnabled, isFallback } = ctx;
        let data = memoryCache == null ? void 0 : memoryCache.get(key);
        if (this.debug) {
            console.log('get', key, tags, kind, !!data);
        }
        // let's check the disk for seed data
        if (!data && process.env.NEXT_RUNTIME !== 'edge') {
            if (kind === _responsecache.IncrementalCacheKind.APP_ROUTE) {
                try {
                    const filePath = this.getFilePath(`${key}.body`, _responsecache.IncrementalCacheKind.APP_ROUTE);
                    const fileData = await this.fs.readFile(filePath);
                    const { mtime } = await this.fs.stat(filePath);
                    const meta = JSON.parse(await this.fs.readFile(filePath.replace(/\.body$/, _constants.NEXT_META_SUFFIX), 'utf8'));
                    const cacheEntry = {
                        lastModified: mtime.getTime(),
                        value: {
                            kind: _responsecache.CachedRouteKind.APP_ROUTE,
                            body: fileData,
                            headers: meta.headers,
                            status: meta.status
                        }
                    };
                    return cacheEntry;
                } catch  {
                    return null;
                }
            }
            try {
                const filePath = this.getFilePath(kind === _responsecache.IncrementalCacheKind.FETCH ? key : `${key}.html`, kind);
                const fileData = await this.fs.readFile(filePath, 'utf8');
                const { mtime } = await this.fs.stat(filePath);
                if (kind === _responsecache.IncrementalCacheKind.FETCH) {
                    var _data_value3;
                    if (!this.flushToDisk) return null;
                    const lastModified = mtime.getTime();
                    const parsedData = JSON.parse(fileData);
                    data = {
                        lastModified,
                        value: parsedData
                    };
                    if (((_data_value3 = data.value) == null ? void 0 : _data_value3.kind) === _responsecache.CachedRouteKind.FETCH) {
                        var _data_value4;
                        const storedTags = (_data_value4 = data.value) == null ? void 0 : _data_value4.tags;
                        // update stored tags if a new one is being added
                        // TODO: remove this when we can send the tags
                        // via header on GET same as SET
                        if (!(tags == null ? void 0 : tags.every((tag)=>storedTags == null ? void 0 : storedTags.includes(tag)))) {
                            if (this.debug) {
                                console.log('tags vs storedTags mismatch', tags, storedTags);
                            }
                            await this.set(key, data.value, {
                                tags,
                                isRoutePPREnabled
                            });
                        }
                    }
                } else if (kind === _responsecache.IncrementalCacheKind.APP_PAGE) {
                    // We try to load the metadata file, but if it fails, we don't
                    // error. We also don't load it if this is a fallback.
                    let meta;
                    try {
                        meta = JSON.parse(await this.fs.readFile(filePath.replace(/\.html$/, _constants.NEXT_META_SUFFIX), 'utf8'));
                    } catch  {}
                    let maybeSegmentData;
                    if (meta == null ? void 0 : meta.segmentPaths) {
                        // Collect all the segment data for this page.
                        // TODO: To optimize file system reads, we should consider creating
                        // separate cache entries for each segment, rather than storing them
                        // all on the page's entry. Though the behavior is
                        // identical regardless.
                        const segmentData = new Map();
                        maybeSegmentData = segmentData;
                        const segmentsDir = key + _constants.RSC_SEGMENTS_DIR_SUFFIX;
                        await Promise.all(meta.segmentPaths.map(async (segmentPath)=>{
                            const segmentDataFilePath = this.getFilePath(segmentPath === '/' ? segmentsDir + '/_index' + _constants.RSC_SEGMENT_SUFFIX : segmentsDir + segmentPath + _constants.RSC_SEGMENT_SUFFIX, _responsecache.IncrementalCacheKind.APP_PAGE);
                            try {
                                segmentData.set(segmentPath, await this.fs.readFile(segmentDataFilePath));
                            } catch  {
                            // This shouldn't happen, but if for some reason we fail to
                            // load a segment from the filesystem, treat it the same as if
                            // the segment is dynamic and does not have a prefetch.
                            }
                        }));
                    }
                    let rscData;
                    if (!isFallback) {
                        rscData = await this.fs.readFile(this.getFilePath(`${key}${isRoutePPREnabled ? _constants.RSC_PREFETCH_SUFFIX : _constants.RSC_SUFFIX}`, _responsecache.IncrementalCacheKind.APP_PAGE));
                    }
                    data = {
                        lastModified: mtime.getTime(),
                        value: {
                            kind: _responsecache.CachedRouteKind.APP_PAGE,
                            html: fileData,
                            rscData,
                            postponed: meta == null ? void 0 : meta.postponed,
                            headers: meta == null ? void 0 : meta.headers,
                            status: meta == null ? void 0 : meta.status,
                            segmentData: maybeSegmentData
                        }
                    };
                } else if (kind === _responsecache.IncrementalCacheKind.PAGES) {
                    let meta;
                    let pageData = {};
                    if (!isFallback) {
                        pageData = JSON.parse(await this.fs.readFile(this.getFilePath(`${key}${_constants.NEXT_DATA_SUFFIX}`, _responsecache.IncrementalCacheKind.PAGES), 'utf8'));
                    }
                    data = {
                        lastModified: mtime.getTime(),
                        value: {
                            kind: _responsecache.CachedRouteKind.PAGES,
                            html: fileData,
                            pageData,
                            headers: meta == null ? void 0 : meta.headers,
                            status: meta == null ? void 0 : meta.status
                        }
                    };
                } else {
                    throw new Error(`Invariant: Unexpected route kind ${kind} in file system cache.`);
                }
                if (data) {
                    memoryCache == null ? void 0 : memoryCache.set(key, data);
                }
            } catch  {
                return null;
            }
        }
        if ((data == null ? void 0 : (_data_value = data.value) == null ? void 0 : _data_value.kind) === _responsecache.CachedRouteKind.APP_PAGE || (data == null ? void 0 : (_data_value1 = data.value) == null ? void 0 : _data_value1.kind) === _responsecache.CachedRouteKind.PAGES) {
            var _data_value_headers;
            let cacheTags;
            const tagsHeader = (_data_value_headers = data.value.headers) == null ? void 0 : _data_value_headers[_constants.NEXT_CACHE_TAGS_HEADER];
            if (typeof tagsHeader === 'string') {
                cacheTags = tagsHeader.split(',');
            }
            if (cacheTags == null ? void 0 : cacheTags.length) {
                const isStale = cacheTags.some((tag)=>{
                    var _tagsManifest_items_tag;
                    return (_tagsmanifestexternal.tagsManifest == null ? void 0 : (_tagsManifest_items_tag = _tagsmanifestexternal.tagsManifest.items[tag]) == null ? void 0 : _tagsManifest_items_tag.revalidatedAt) && (_tagsmanifestexternal.tagsManifest == null ? void 0 : _tagsmanifestexternal.tagsManifest.items[tag].revalidatedAt) >= ((data == null ? void 0 : data.lastModified) || Date.now());
                });
                // we trigger a blocking validation if an ISR page
                // had a tag revalidated, if we want to be a background
                // revalidation instead we return data.lastModified = -1
                if (isStale) {
                    return null;
                }
            }
        } else if ((data == null ? void 0 : (_data_value2 = data.value) == null ? void 0 : _data_value2.kind) === _responsecache.CachedRouteKind.FETCH) {
            const combinedTags = [
                ...tags || [],
                ...softTags || []
            ];
            const wasRevalidated = combinedTags.some((tag)=>{
                var _tagsManifest_items_tag;
                if (this.revalidatedTags.includes(tag)) {
                    return true;
                }
                return (_tagsmanifestexternal.tagsManifest == null ? void 0 : (_tagsManifest_items_tag = _tagsmanifestexternal.tagsManifest.items[tag]) == null ? void 0 : _tagsManifest_items_tag.revalidatedAt) && (_tagsmanifestexternal.tagsManifest == null ? void 0 : _tagsmanifestexternal.tagsManifest.items[tag].revalidatedAt) >= ((data == null ? void 0 : data.lastModified) || Date.now());
            });
            // When revalidate tag is called we don't return
            // stale data so it's updated right away
            if (wasRevalidated) {
                data = undefined;
            }
        }
        return data ?? null;
    }
    async set(...args) {
        const [key, data, ctx] = args;
        const { isFallback } = ctx;
        memoryCache == null ? void 0 : memoryCache.set(key, {
            value: data,
            lastModified: Date.now()
        });
        if (this.debug) {
            console.log('set', key);
        }
        if (!this.flushToDisk || !data) return;
        if (data.kind === _responsecache.CachedRouteKind.APP_ROUTE) {
            const filePath = this.getFilePath(`${key}.body`, _responsecache.IncrementalCacheKind.APP_ROUTE);
            await this.fs.mkdir(_path.default.dirname(filePath));
            await this.fs.writeFile(filePath, data.body);
            const meta = {
                headers: data.headers,
                status: data.status,
                postponed: undefined,
                segmentPaths: undefined
            };
            await this.fs.writeFile(filePath.replace(/\.body$/, _constants.NEXT_META_SUFFIX), JSON.stringify(meta, null, 2));
        } else if (data.kind === _responsecache.CachedRouteKind.PAGES || data.kind === _responsecache.CachedRouteKind.APP_PAGE) {
            const isAppPath = data.kind === _responsecache.CachedRouteKind.APP_PAGE;
            const htmlPath = this.getFilePath(`${key}.html`, isAppPath ? _responsecache.IncrementalCacheKind.APP_PAGE : _responsecache.IncrementalCacheKind.PAGES);
            await this.fs.mkdir(_path.default.dirname(htmlPath));
            await this.fs.writeFile(htmlPath, data.html);
            // Fallbacks don't generate a data file.
            if (!isFallback) {
                await this.fs.writeFile(this.getFilePath(`${key}${isAppPath ? ctx.isRoutePPREnabled ? _constants.RSC_PREFETCH_SUFFIX : _constants.RSC_SUFFIX : _constants.NEXT_DATA_SUFFIX}`, isAppPath ? _responsecache.IncrementalCacheKind.APP_PAGE : _responsecache.IncrementalCacheKind.PAGES), isAppPath ? data.rscData : JSON.stringify(data.pageData));
            }
            if ((data == null ? void 0 : data.kind) === _responsecache.CachedRouteKind.APP_PAGE) {
                const meta = {
                    headers: data.headers,
                    status: data.status,
                    postponed: data.postponed,
                    segmentPaths: undefined
                };
                await this.fs.writeFile(htmlPath.replace(/\.html$/, _constants.NEXT_META_SUFFIX), JSON.stringify(meta));
            }
        } else if (data.kind === _responsecache.CachedRouteKind.FETCH) {
            const filePath = this.getFilePath(key, _responsecache.IncrementalCacheKind.FETCH);
            await this.fs.mkdir(_path.default.dirname(filePath));
            await this.fs.writeFile(filePath, JSON.stringify({
                ...data,
                tags: ctx.tags
            }));
        }
    }
    getFilePath(pathname, kind) {
        switch(kind){
            case _responsecache.IncrementalCacheKind.FETCH:
                // we store in .next/cache/fetch-cache so it can be persisted
                // across deploys
                return _path.default.join(this.serverDistDir, '..', 'cache', 'fetch-cache', pathname);
            case _responsecache.IncrementalCacheKind.PAGES:
                return _path.default.join(this.serverDistDir, 'pages', pathname);
            case _responsecache.IncrementalCacheKind.IMAGE:
            case _responsecache.IncrementalCacheKind.APP_PAGE:
            case _responsecache.IncrementalCacheKind.APP_ROUTE:
                return _path.default.join(this.serverDistDir, 'app', pathname);
            default:
                throw new Error(`Unexpected file path kind: ${kind}`);
        }
    }
}

//# sourceMappingURL=file-system-cache.js.map