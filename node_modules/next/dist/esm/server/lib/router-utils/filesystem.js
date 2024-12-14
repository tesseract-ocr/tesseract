import path from 'path';
import fs from 'fs/promises';
import * as Log from '../../../build/output/log';
import setupDebug from 'next/dist/compiled/debug';
import { LRUCache } from '../lru-cache';
import loadCustomRoutes from '../../../lib/load-custom-routes';
import { modifyRouteRegex } from '../../../lib/redirect-status';
import { FileType, fileExists } from '../../../lib/file-exists';
import { recursiveReadDir } from '../../../lib/recursive-readdir';
import { isDynamicRoute } from '../../../shared/lib/router/utils';
import { escapeStringRegexp } from '../../../shared/lib/escape-regexp';
import { getPathMatch } from '../../../shared/lib/router/utils/path-match';
import { getRouteRegex } from '../../../shared/lib/router/utils/route-regex';
import { getRouteMatcher } from '../../../shared/lib/router/utils/route-matcher';
import { pathHasPrefix } from '../../../shared/lib/router/utils/path-has-prefix';
import { normalizeLocalePath } from '../../../shared/lib/i18n/normalize-locale-path';
import { removePathPrefix } from '../../../shared/lib/router/utils/remove-path-prefix';
import { getMiddlewareRouteMatcher } from '../../../shared/lib/router/utils/middleware-route-matcher';
import { APP_PATH_ROUTES_MANIFEST, BUILD_ID_FILE, MIDDLEWARE_MANIFEST, PAGES_MANIFEST, PRERENDER_MANIFEST, ROUTES_MANIFEST } from '../../../shared/lib/constants';
import { normalizePathSep } from '../../../shared/lib/page-path/normalize-path-sep';
import { normalizeMetadataRoute } from '../../../lib/metadata/get-metadata-route';
import { RSCPathnameNormalizer } from '../../normalizers/request/rsc';
import { PrefetchRSCPathnameNormalizer } from '../../normalizers/request/prefetch-rsc';
import { encodeURIPath } from '../../../shared/lib/encode-uri-path';
const debug = setupDebug('next:router-server:filesystem');
export const buildCustomRoute = (type, item, basePath, caseSensitive)=>{
    const restrictedRedirectPaths = [
        '/_next'
    ].map((p)=>basePath ? `${basePath}${p}` : p);
    const match = getPathMatch(item.source, {
        strict: true,
        removeUnnamedParams: true,
        regexModifier: !item.internal ? (regex)=>modifyRouteRegex(regex, type === 'redirect' ? restrictedRedirectPaths : undefined) : undefined,
        sensitive: caseSensitive
    });
    return {
        ...item,
        ...type === 'rewrite' ? {
            check: true
        } : {},
        match
    };
};
export async function setupFsCheck(opts) {
    const getItemsLru = !opts.dev ? new LRUCache(1024 * 1024, function length(value) {
        if (!value) return 0;
        return (value.fsPath || '').length + value.itemPath.length + value.type.length;
    }) : undefined;
    // routes that have _next/data endpoints (SSG/SSP)
    const nextDataRoutes = new Set();
    const publicFolderItems = new Set();
    const nextStaticFolderItems = new Set();
    const legacyStaticFolderItems = new Set();
    const appFiles = new Set();
    const pageFiles = new Set();
    let dynamicRoutes = [];
    let middlewareMatcher = ()=>false;
    const distDir = path.join(opts.dir, opts.config.distDir);
    const publicFolderPath = path.join(opts.dir, 'public');
    const nextStaticFolderPath = path.join(distDir, 'static');
    const legacyStaticFolderPath = path.join(opts.dir, 'static');
    let customRoutes = {
        redirects: [],
        rewrites: {
            beforeFiles: [],
            afterFiles: [],
            fallback: []
        },
        headers: []
    };
    let buildId = 'development';
    let prerenderManifest;
    if (!opts.dev) {
        var _middlewareManifest_middleware_, _middlewareManifest_middleware;
        const buildIdPath = path.join(opts.dir, opts.config.distDir, BUILD_ID_FILE);
        try {
            buildId = await fs.readFile(buildIdPath, 'utf8');
        } catch (err) {
            if (err.code !== 'ENOENT') throw err;
            throw new Error(`Could not find a production build in the '${opts.config.distDir}' directory. Try building your app with 'next build' before starting the production server. https://nextjs.org/docs/messages/production-start-no-build-id`);
        }
        try {
            for (const file of (await recursiveReadDir(publicFolderPath))){
                // Ensure filename is encoded and normalized.
                publicFolderItems.add(encodeURIPath(normalizePathSep(file)));
            }
        } catch (err) {
            if (err.code !== 'ENOENT') {
                throw err;
            }
        }
        try {
            for (const file of (await recursiveReadDir(legacyStaticFolderPath))){
                // Ensure filename is encoded and normalized.
                legacyStaticFolderItems.add(encodeURIPath(normalizePathSep(file)));
            }
            Log.warn(`The static directory has been deprecated in favor of the public directory. https://nextjs.org/docs/messages/static-dir-deprecated`);
        } catch (err) {
            if (err.code !== 'ENOENT') {
                throw err;
            }
        }
        try {
            for (const file of (await recursiveReadDir(nextStaticFolderPath))){
                // Ensure filename is encoded and normalized.
                nextStaticFolderItems.add(path.posix.join('/_next/static', encodeURIPath(normalizePathSep(file))));
            }
        } catch (err) {
            if (opts.config.output !== 'standalone') throw err;
        }
        const routesManifestPath = path.join(distDir, ROUTES_MANIFEST);
        const prerenderManifestPath = path.join(distDir, PRERENDER_MANIFEST);
        const middlewareManifestPath = path.join(distDir, 'server', MIDDLEWARE_MANIFEST);
        const pagesManifestPath = path.join(distDir, 'server', PAGES_MANIFEST);
        const appRoutesManifestPath = path.join(distDir, APP_PATH_ROUTES_MANIFEST);
        const routesManifest = JSON.parse(await fs.readFile(routesManifestPath, 'utf8'));
        prerenderManifest = JSON.parse(await fs.readFile(prerenderManifestPath, 'utf8'));
        const middlewareManifest = JSON.parse(await fs.readFile(middlewareManifestPath, 'utf8').catch(()=>'{}'));
        const pagesManifest = JSON.parse(await fs.readFile(pagesManifestPath, 'utf8'));
        const appRoutesManifest = JSON.parse(await fs.readFile(appRoutesManifestPath, 'utf8').catch(()=>'{}'));
        for (const key of Object.keys(pagesManifest)){
            // ensure the non-locale version is in the set
            if (opts.config.i18n) {
                pageFiles.add(normalizeLocalePath(key, opts.config.i18n.locales).pathname);
            } else {
                pageFiles.add(key);
            }
        }
        for (const key of Object.keys(appRoutesManifest)){
            appFiles.add(appRoutesManifest[key]);
        }
        const escapedBuildId = escapeStringRegexp(buildId);
        for (const route of routesManifest.dataRoutes){
            if (isDynamicRoute(route.page)) {
                const routeRegex = getRouteRegex(route.page);
                dynamicRoutes.push({
                    ...route,
                    regex: routeRegex.re.toString(),
                    match: getRouteMatcher({
                        // TODO: fix this in the manifest itself, must also be fixed in
                        // upstream builder that relies on this
                        re: opts.config.i18n ? new RegExp(route.dataRouteRegex.replace(`/${escapedBuildId}/`, `/${escapedBuildId}/(?<nextLocale>[^/]+?)/`)) : new RegExp(route.dataRouteRegex),
                        groups: routeRegex.groups
                    })
                });
            }
            nextDataRoutes.add(route.page);
        }
        for (const route of routesManifest.dynamicRoutes){
            dynamicRoutes.push({
                ...route,
                match: getRouteMatcher(getRouteRegex(route.page))
            });
        }
        if ((_middlewareManifest_middleware = middlewareManifest.middleware) == null ? void 0 : (_middlewareManifest_middleware_ = _middlewareManifest_middleware['/']) == null ? void 0 : _middlewareManifest_middleware_.matchers) {
            var _middlewareManifest_middleware_1, _middlewareManifest_middleware1;
            middlewareMatcher = getMiddlewareRouteMatcher((_middlewareManifest_middleware1 = middlewareManifest.middleware) == null ? void 0 : (_middlewareManifest_middleware_1 = _middlewareManifest_middleware1['/']) == null ? void 0 : _middlewareManifest_middleware_1.matchers);
        }
        customRoutes = {
            redirects: routesManifest.redirects,
            rewrites: routesManifest.rewrites ? Array.isArray(routesManifest.rewrites) ? {
                beforeFiles: [],
                afterFiles: routesManifest.rewrites,
                fallback: []
            } : routesManifest.rewrites : {
                beforeFiles: [],
                afterFiles: [],
                fallback: []
            },
            headers: routesManifest.headers
        };
    } else {
        // dev handling
        customRoutes = await loadCustomRoutes(opts.config);
        prerenderManifest = {
            version: 4,
            routes: {},
            dynamicRoutes: {},
            notFoundRoutes: [],
            preview: {
                previewModeId: require('crypto').randomBytes(16).toString('hex'),
                previewModeSigningKey: require('crypto').randomBytes(32).toString('hex'),
                previewModeEncryptionKey: require('crypto').randomBytes(32).toString('hex')
            }
        };
    }
    const headers = customRoutes.headers.map((item)=>buildCustomRoute('header', item, opts.config.basePath, opts.config.experimental.caseSensitiveRoutes));
    const redirects = customRoutes.redirects.map((item)=>buildCustomRoute('redirect', item, opts.config.basePath, opts.config.experimental.caseSensitiveRoutes));
    const rewrites = {
        beforeFiles: customRoutes.rewrites.beforeFiles.map((item)=>buildCustomRoute('before_files_rewrite', item)),
        afterFiles: customRoutes.rewrites.afterFiles.map((item)=>buildCustomRoute('rewrite', item, opts.config.basePath, opts.config.experimental.caseSensitiveRoutes)),
        fallback: customRoutes.rewrites.fallback.map((item)=>buildCustomRoute('rewrite', item, opts.config.basePath, opts.config.experimental.caseSensitiveRoutes))
    };
    const { i18n } = opts.config;
    const handleLocale = (pathname, locales)=>{
        let locale;
        if (i18n) {
            const i18nResult = normalizeLocalePath(pathname, locales || i18n.locales);
            pathname = i18nResult.pathname;
            locale = i18nResult.detectedLocale;
        }
        return {
            locale,
            pathname
        };
    };
    debug('nextDataRoutes', nextDataRoutes);
    debug('dynamicRoutes', dynamicRoutes);
    debug('pageFiles', pageFiles);
    debug('appFiles', appFiles);
    let ensureFn;
    const normalizers = {
        // Because we can't know if the app directory is enabled or not at this
        // stage, we assume that it is.
        rsc: new RSCPathnameNormalizer(),
        prefetchRSC: opts.config.experimental.ppr ? new PrefetchRSCPathnameNormalizer() : undefined
    };
    return {
        headers,
        rewrites,
        redirects,
        buildId,
        handleLocale,
        appFiles,
        pageFiles,
        dynamicRoutes,
        nextDataRoutes,
        exportPathMapRoutes: undefined,
        devVirtualFsItems: new Set(),
        prerenderManifest,
        middlewareMatcher: middlewareMatcher,
        ensureCallback (fn) {
            ensureFn = fn;
        },
        async getItem (itemPath) {
            const originalItemPath = itemPath;
            const itemKey = originalItemPath;
            const lruResult = getItemsLru == null ? void 0 : getItemsLru.get(itemKey);
            if (lruResult) {
                return lruResult;
            }
            const { basePath } = opts.config;
            const hasBasePath = pathHasPrefix(itemPath, basePath);
            // Return null if path doesn't start with basePath
            if (basePath && !hasBasePath) {
                return null;
            }
            // Remove basePath if it exists.
            if (basePath && hasBasePath) {
                itemPath = removePathPrefix(itemPath, basePath) || '/';
            }
            // Simulate minimal mode requests by normalizing RSC and postponed
            // requests.
            if (opts.minimalMode) {
                var _normalizers_prefetchRSC;
                if ((_normalizers_prefetchRSC = normalizers.prefetchRSC) == null ? void 0 : _normalizers_prefetchRSC.match(itemPath)) {
                    itemPath = normalizers.prefetchRSC.normalize(itemPath, true);
                } else if (normalizers.rsc.match(itemPath)) {
                    itemPath = normalizers.rsc.normalize(itemPath, true);
                }
            }
            if (itemPath !== '/' && itemPath.endsWith('/')) {
                itemPath = itemPath.substring(0, itemPath.length - 1);
            }
            let decodedItemPath = itemPath;
            try {
                decodedItemPath = decodeURIComponent(itemPath);
            } catch  {}
            if (itemPath === '/_next/image') {
                return {
                    itemPath,
                    type: 'nextImage'
                };
            }
            const itemsToCheck = [
                [
                    this.devVirtualFsItems,
                    'devVirtualFsItem'
                ],
                [
                    nextStaticFolderItems,
                    'nextStaticFolder'
                ],
                [
                    legacyStaticFolderItems,
                    'legacyStaticFolder'
                ],
                [
                    publicFolderItems,
                    'publicFolder'
                ],
                [
                    appFiles,
                    'appFile'
                ],
                [
                    pageFiles,
                    'pageFile'
                ]
            ];
            for (let [items, type] of itemsToCheck){
                let locale;
                let curItemPath = itemPath;
                let curDecodedItemPath = decodedItemPath;
                const isDynamicOutput = type === 'pageFile' || type === 'appFile';
                if (i18n) {
                    var _i18n_domains;
                    const localeResult = handleLocale(itemPath, // legacy behavior allows visiting static assets under
                    // default locale but no other locale
                    isDynamicOutput ? undefined : [
                        i18n == null ? void 0 : i18n.defaultLocale,
                        // default locales from domains need to be matched too
                        ...((_i18n_domains = i18n.domains) == null ? void 0 : _i18n_domains.map((item)=>item.defaultLocale)) || []
                    ]);
                    if (localeResult.pathname !== curItemPath) {
                        curItemPath = localeResult.pathname;
                        locale = localeResult.locale;
                        try {
                            curDecodedItemPath = decodeURIComponent(curItemPath);
                        } catch  {}
                    }
                }
                if (type === 'legacyStaticFolder') {
                    if (!pathHasPrefix(curItemPath, '/static')) {
                        continue;
                    }
                    curItemPath = curItemPath.substring('/static'.length);
                    try {
                        curDecodedItemPath = decodeURIComponent(curItemPath);
                    } catch  {}
                }
                if (type === 'nextStaticFolder' && !pathHasPrefix(curItemPath, '/_next/static')) {
                    continue;
                }
                const nextDataPrefix = `/_next/data/${buildId}/`;
                if (type === 'pageFile' && curItemPath.startsWith(nextDataPrefix) && curItemPath.endsWith('.json')) {
                    items = nextDataRoutes;
                    // remove _next/data/<build-id> prefix
                    curItemPath = curItemPath.substring(nextDataPrefix.length - 1);
                    // remove .json postfix
                    curItemPath = curItemPath.substring(0, curItemPath.length - '.json'.length);
                    const curLocaleResult = handleLocale(curItemPath);
                    curItemPath = curLocaleResult.pathname === '/index' ? '/' : curLocaleResult.pathname;
                    locale = curLocaleResult.locale;
                    try {
                        curDecodedItemPath = decodeURIComponent(curItemPath);
                    } catch  {}
                }
                let matchedItem = items.has(curItemPath);
                // check decoded variant as well
                if (!matchedItem && !opts.dev) {
                    matchedItem = items.has(curDecodedItemPath);
                    if (matchedItem) curItemPath = curDecodedItemPath;
                    else {
                        // x-ref: https://github.com/vercel/next.js/issues/54008
                        // There're cases that urls get decoded before requests, we should support both encoded and decoded ones.
                        // e.g. nginx could decode the proxy urls, the below ones should be treated as the same:
                        // decoded version: `/_next/static/chunks/pages/blog/[slug]-d4858831b91b69f6.js`
                        // encoded version: `/_next/static/chunks/pages/blog/%5Bslug%5D-d4858831b91b69f6.js`
                        try {
                            // encode the special characters in the path and retrieve again to determine if path exists.
                            const encodedCurItemPath = encodeURIPath(curItemPath);
                            matchedItem = items.has(encodedCurItemPath);
                        } catch  {}
                    }
                }
                if (matchedItem || opts.dev) {
                    let fsPath;
                    let itemsRoot;
                    switch(type){
                        case 'nextStaticFolder':
                            {
                                itemsRoot = nextStaticFolderPath;
                                curItemPath = curItemPath.substring('/_next/static'.length);
                                break;
                            }
                        case 'legacyStaticFolder':
                            {
                                itemsRoot = legacyStaticFolderPath;
                                break;
                            }
                        case 'publicFolder':
                            {
                                itemsRoot = publicFolderPath;
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    if (itemsRoot && curItemPath) {
                        fsPath = path.posix.join(itemsRoot, curItemPath);
                    }
                    // dynamically check fs in development so we don't
                    // have to wait on the watcher
                    if (!matchedItem && opts.dev) {
                        const isStaticAsset = [
                            'nextStaticFolder',
                            'publicFolder',
                            'legacyStaticFolder'
                        ].includes(type);
                        if (isStaticAsset && itemsRoot) {
                            let found = fsPath && await fileExists(fsPath, FileType.File);
                            if (!found) {
                                try {
                                    // In dev, we ensure encoded paths match
                                    // decoded paths on the filesystem so check
                                    // that variation as well
                                    const tempItemPath = decodeURIComponent(curItemPath);
                                    fsPath = path.posix.join(itemsRoot, tempItemPath);
                                    found = await fileExists(fsPath, FileType.File);
                                } catch  {}
                                if (!found) {
                                    continue;
                                }
                            }
                        } else if (type === 'pageFile' || type === 'appFile') {
                            var _ensureFn;
                            const isAppFile = type === 'appFile';
                            if (ensureFn && await ((_ensureFn = ensureFn({
                                type,
                                itemPath: isAppFile ? normalizeMetadataRoute(curItemPath) : curItemPath
                            })) == null ? void 0 : _ensureFn.catch(()=>'ENSURE_FAILED')) === 'ENSURE_FAILED') {
                                continue;
                            }
                        } else {
                            continue;
                        }
                    }
                    // i18n locales aren't matched for app dir
                    if (type === 'appFile' && locale && locale !== (i18n == null ? void 0 : i18n.defaultLocale)) {
                        continue;
                    }
                    const itemResult = {
                        type,
                        fsPath,
                        locale,
                        itemsRoot,
                        itemPath: curItemPath
                    };
                    getItemsLru == null ? void 0 : getItemsLru.set(itemKey, itemResult);
                    return itemResult;
                }
            }
            getItemsLru == null ? void 0 : getItemsLru.set(itemKey, null);
            return null;
        },
        getDynamicRoutes () {
            // this should include data routes
            return this.dynamicRoutes;
        },
        getMiddlewareMatchers () {
            return this.middlewareMatcher;
        }
    };
}

//# sourceMappingURL=filesystem.js.map