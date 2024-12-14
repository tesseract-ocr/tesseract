import path from 'path';
import { IncrementalCache } from '../../server/lib/incremental-cache';
import { hasNextSupport } from '../../server/ci-info';
import { nodeFs } from '../../server/lib/node-fs-methods';
import { interopDefault } from '../../lib/interop-default';
import { formatDynamicImportPath } from '../../lib/format-dynamic-import-path';
export async function createIncrementalCache({ cacheHandler, dynamicIO, cacheMaxMemorySize, fetchCacheKeyPrefix, distDir, dir, flushToDisk, cacheHandlers }) {
    // Custom cache handler overrides.
    let CacheHandler;
    if (cacheHandler) {
        CacheHandler = interopDefault(await import(formatDynamicImportPath(dir, cacheHandler)).then((mod)=>mod.default || mod));
    }
    if (!globalThis.__nextCacheHandlers && cacheHandlers) {
        ;
        globalThis.__nextCacheHandlers = {};
        for (const key of Object.keys(cacheHandlers)){
            if (cacheHandlers[key]) {
                ;
                globalThis.__nextCacheHandlers[key] = interopDefault(await import(formatDynamicImportPath(dir, cacheHandlers[key])).then((mod)=>mod.default || mod));
            }
        }
    }
    const incrementalCache = new IncrementalCache({
        dev: false,
        requestHeaders: {},
        flushToDisk,
        dynamicIO,
        fetchCache: true,
        maxMemoryCacheSize: cacheMaxMemorySize,
        fetchCacheKeyPrefix,
        getPrerenderManifest: ()=>({
                version: 4,
                routes: {},
                dynamicRoutes: {},
                preview: {
                    previewModeEncryptionKey: '',
                    previewModeId: '',
                    previewModeSigningKey: ''
                },
                notFoundRoutes: []
            }),
        fs: nodeFs,
        serverDistDir: path.join(distDir, 'server'),
        CurCacheHandler: CacheHandler,
        minimalMode: hasNextSupport
    });
    globalThis.__incrementalCache = incrementalCache;
    return incrementalCache;
}

//# sourceMappingURL=create-incremental-cache.js.map