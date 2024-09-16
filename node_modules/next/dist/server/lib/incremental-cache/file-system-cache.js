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
const _lrucache = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/lru-cache"));
const _path = /*#__PURE__*/ _interop_require_default(require("../../../shared/lib/isomorphic/path"));
const _constants = require("../../../lib/constants");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
let memoryCache;
let tagsManifest;
class FileSystemCache {
    constructor(ctx){
        this.fs = ctx.fs;
        this.flushToDisk = ctx.flushToDisk;
        this.serverDistDir = ctx.serverDistDir;
        this.appDir = !!ctx._appDir;
        this.pagesDir = !!ctx._pagesDir;
        this.revalidatedTags = ctx.revalidatedTags;
        this.experimental = ctx.experimental;
        this.debug = !!process.env.NEXT_PRIVATE_DEBUG_CACHE;
        if (ctx.maxMemoryCacheSize && !memoryCache) {
            if (this.debug) {
                console.log("using memory store for fetch cache");
            }
            memoryCache = new _lrucache.default({
                max: ctx.maxMemoryCacheSize,
                length ({ value }) {
                    var _JSON_stringify;
                    if (!value) {
                        return 25;
                    } else if (value.kind === "REDIRECT") {
                        return JSON.stringify(value.props).length;
                    } else if (value.kind === "IMAGE") {
                        throw new Error("invariant image should not be incremental-cache");
                    } else if (value.kind === "FETCH") {
                        return JSON.stringify(value.data || "").length;
                    } else if (value.kind === "ROUTE") {
                        return value.body.length;
                    }
                    // rough estimate of size of cache value
                    return value.html.length + (((_JSON_stringify = JSON.stringify(value.pageData)) == null ? void 0 : _JSON_stringify.length) || 0);
                }
            });
        } else if (this.debug) {
            console.log("not using memory store for fetch cache");
        }
        if (this.serverDistDir && this.fs) {
            this.tagsManifestPath = _path.default.join(this.serverDistDir, "..", "cache", "fetch-cache", "tags-manifest.json");
            this.loadTagsManifest();
        }
    }
    resetRequestCache() {}
    loadTagsManifest() {
        if (!this.tagsManifestPath || !this.fs || tagsManifest) return;
        try {
            tagsManifest = JSON.parse(this.fs.readFileSync(this.tagsManifestPath, "utf8"));
        } catch (err) {
            tagsManifest = {
                version: 1,
                items: {}
            };
        }
        if (this.debug) console.log("loadTagsManifest", tagsManifest);
    }
    async revalidateTag(...args) {
        let [tags] = args;
        tags = typeof tags === "string" ? [
            tags
        ] : tags;
        if (this.debug) {
            console.log("revalidateTag", tags);
        }
        if (tags.length === 0) {
            return;
        }
        // we need to ensure the tagsManifest is refreshed
        // since separate workers can be updating it at the same
        // time and we can't flush out of sync data
        await this.loadTagsManifest();
        if (!tagsManifest || !this.tagsManifestPath) {
            return;
        }
        for (const tag of tags){
            const data = tagsManifest.items[tag] || {};
            data.revalidatedAt = Date.now();
            tagsManifest.items[tag] = data;
        }
        try {
            await this.fs.mkdir(_path.default.dirname(this.tagsManifestPath));
            await this.fs.writeFile(this.tagsManifestPath, JSON.stringify(tagsManifest || {}));
            if (this.debug) {
                console.log("Updated tags manifest", tagsManifest);
            }
        } catch (err) {
            console.warn("Failed to update tags manifest.", err);
        }
    }
    async get(...args) {
        var _data_value, _data_value1;
        const [key, ctx = {}] = args;
        const { tags, softTags, kindHint } = ctx;
        let data = memoryCache == null ? void 0 : memoryCache.get(key);
        if (this.debug) {
            console.log("get", key, tags, kindHint, !!data);
        }
        // let's check the disk for seed data
        if (!data && process.env.NEXT_RUNTIME !== "edge") {
            try {
                const filePath = this.getFilePath(`${key}.body`, "app");
                const fileData = await this.fs.readFile(filePath);
                const { mtime } = await this.fs.stat(filePath);
                const meta = JSON.parse(await this.fs.readFile(filePath.replace(/\.body$/, _constants.NEXT_META_SUFFIX), "utf8"));
                const cacheEntry = {
                    lastModified: mtime.getTime(),
                    value: {
                        kind: "ROUTE",
                        body: fileData,
                        headers: meta.headers,
                        status: meta.status
                    }
                };
                return cacheEntry;
            } catch (_) {
            // no .meta data for the related key
            }
            try {
                // Determine the file kind if we didn't know it already.
                let kind = kindHint;
                if (!kind) {
                    kind = this.detectFileKind(`${key}.html`);
                }
                const isAppPath = kind === "app";
                const filePath = this.getFilePath(kind === "fetch" ? key : `${key}.html`, kind);
                const fileData = await this.fs.readFile(filePath, "utf8");
                const { mtime } = await this.fs.stat(filePath);
                if (kind === "fetch" && this.flushToDisk) {
                    var _data_value2;
                    const lastModified = mtime.getTime();
                    const parsedData = JSON.parse(fileData);
                    data = {
                        lastModified,
                        value: parsedData
                    };
                    if (((_data_value2 = data.value) == null ? void 0 : _data_value2.kind) === "FETCH") {
                        var _data_value3;
                        const storedTags = (_data_value3 = data.value) == null ? void 0 : _data_value3.tags;
                        // update stored tags if a new one is being added
                        // TODO: remove this when we can send the tags
                        // via header on GET same as SET
                        if (!(tags == null ? void 0 : tags.every((tag)=>storedTags == null ? void 0 : storedTags.includes(tag)))) {
                            if (this.debug) {
                                console.log("tags vs storedTags mismatch", tags, storedTags);
                            }
                            await this.set(key, data.value, {
                                tags
                            });
                        }
                    }
                } else {
                    const pageData = isAppPath ? await this.fs.readFile(this.getFilePath(`${key}${this.experimental.ppr ? _constants.RSC_PREFETCH_SUFFIX : _constants.RSC_SUFFIX}`, "app"), "utf8") : JSON.parse(await this.fs.readFile(this.getFilePath(`${key}${_constants.NEXT_DATA_SUFFIX}`, "pages"), "utf8"));
                    let meta;
                    if (isAppPath) {
                        try {
                            meta = JSON.parse(await this.fs.readFile(filePath.replace(/\.html$/, _constants.NEXT_META_SUFFIX), "utf8"));
                        } catch  {}
                    }
                    data = {
                        lastModified: mtime.getTime(),
                        value: {
                            kind: "PAGE",
                            html: fileData,
                            pageData,
                            postponed: meta == null ? void 0 : meta.postponed,
                            headers: meta == null ? void 0 : meta.headers,
                            status: meta == null ? void 0 : meta.status
                        }
                    };
                }
                if (data) {
                    memoryCache == null ? void 0 : memoryCache.set(key, data);
                }
            } catch (_) {
            // unable to get data from disk
            }
        }
        if ((data == null ? void 0 : (_data_value = data.value) == null ? void 0 : _data_value.kind) === "PAGE") {
            var _data_value_headers;
            let cacheTags;
            const tagsHeader = (_data_value_headers = data.value.headers) == null ? void 0 : _data_value_headers[_constants.NEXT_CACHE_TAGS_HEADER];
            if (typeof tagsHeader === "string") {
                cacheTags = tagsHeader.split(",");
            }
            if (cacheTags == null ? void 0 : cacheTags.length) {
                this.loadTagsManifest();
                const isStale = cacheTags.some((tag)=>{
                    var _tagsManifest_items_tag;
                    return (tagsManifest == null ? void 0 : (_tagsManifest_items_tag = tagsManifest.items[tag]) == null ? void 0 : _tagsManifest_items_tag.revalidatedAt) && (tagsManifest == null ? void 0 : tagsManifest.items[tag].revalidatedAt) >= ((data == null ? void 0 : data.lastModified) || Date.now());
                });
                // we trigger a blocking validation if an ISR page
                // had a tag revalidated, if we want to be a background
                // revalidation instead we return data.lastModified = -1
                if (isStale) {
                    data = undefined;
                }
            }
        }
        if (data && (data == null ? void 0 : (_data_value1 = data.value) == null ? void 0 : _data_value1.kind) === "FETCH") {
            this.loadTagsManifest();
            const combinedTags = [
                ...tags || [],
                ...softTags || []
            ];
            const wasRevalidated = combinedTags.some((tag)=>{
                var _tagsManifest_items_tag;
                if (this.revalidatedTags.includes(tag)) {
                    return true;
                }
                return (tagsManifest == null ? void 0 : (_tagsManifest_items_tag = tagsManifest.items[tag]) == null ? void 0 : _tagsManifest_items_tag.revalidatedAt) && (tagsManifest == null ? void 0 : tagsManifest.items[tag].revalidatedAt) >= ((data == null ? void 0 : data.lastModified) || Date.now());
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
        memoryCache == null ? void 0 : memoryCache.set(key, {
            value: data,
            lastModified: Date.now()
        });
        if (this.debug) {
            console.log("set", key);
        }
        if (!this.flushToDisk) return;
        if ((data == null ? void 0 : data.kind) === "ROUTE") {
            const filePath = this.getFilePath(`${key}.body`, "app");
            await this.fs.mkdir(_path.default.dirname(filePath));
            await this.fs.writeFile(filePath, data.body);
            const meta = {
                headers: data.headers,
                status: data.status,
                postponed: undefined
            };
            await this.fs.writeFile(filePath.replace(/\.body$/, _constants.NEXT_META_SUFFIX), JSON.stringify(meta, null, 2));
            return;
        }
        if ((data == null ? void 0 : data.kind) === "PAGE") {
            const isAppPath = typeof data.pageData === "string";
            const htmlPath = this.getFilePath(`${key}.html`, isAppPath ? "app" : "pages");
            await this.fs.mkdir(_path.default.dirname(htmlPath));
            await this.fs.writeFile(htmlPath, data.html);
            await this.fs.writeFile(this.getFilePath(`${key}${isAppPath ? this.experimental.ppr ? _constants.RSC_PREFETCH_SUFFIX : _constants.RSC_SUFFIX : _constants.NEXT_DATA_SUFFIX}`, isAppPath ? "app" : "pages"), isAppPath ? data.pageData : JSON.stringify(data.pageData));
            if (data.headers || data.status) {
                const meta = {
                    headers: data.headers,
                    status: data.status,
                    postponed: data.postponed
                };
                await this.fs.writeFile(htmlPath.replace(/\.html$/, _constants.NEXT_META_SUFFIX), JSON.stringify(meta));
            }
        } else if ((data == null ? void 0 : data.kind) === "FETCH") {
            const filePath = this.getFilePath(key, "fetch");
            await this.fs.mkdir(_path.default.dirname(filePath));
            await this.fs.writeFile(filePath, JSON.stringify({
                ...data,
                tags: ctx.tags
            }));
        }
    }
    detectFileKind(pathname) {
        if (!this.appDir && !this.pagesDir) {
            throw new Error("Invariant: Can't determine file path kind, no page directory enabled");
        }
        // If app directory isn't enabled, then assume it's pages and avoid the fs
        // hit.
        if (!this.appDir && this.pagesDir) {
            return "pages";
        } else if (this.appDir && !this.pagesDir) {
            return "app";
        }
        // If both are enabled, we need to test each in order, starting with
        // `pages`.
        let filePath = this.getFilePath(pathname, "pages");
        if (this.fs.existsSync(filePath)) {
            return "pages";
        }
        filePath = this.getFilePath(pathname, "app");
        if (this.fs.existsSync(filePath)) {
            return "app";
        }
        throw new Error(`Invariant: Unable to determine file path kind for ${pathname}`);
    }
    getFilePath(pathname, kind) {
        switch(kind){
            case "fetch":
                // we store in .next/cache/fetch-cache so it can be persisted
                // across deploys
                return _path.default.join(this.serverDistDir, "..", "cache", "fetch-cache", pathname);
            case "pages":
                return _path.default.join(this.serverDistDir, "pages", pathname);
            case "app":
                return _path.default.join(this.serverDistDir, "app", pathname);
            default:
                throw new Error("Invariant: Can't determine file path kind");
        }
    }
}

//# sourceMappingURL=file-system-cache.js.map