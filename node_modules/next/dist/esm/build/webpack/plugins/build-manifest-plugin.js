import devalue from 'next/dist/compiled/devalue';
import { webpack, sources } from 'next/dist/compiled/webpack/webpack';
import { BUILD_MANIFEST, MIDDLEWARE_BUILD_MANIFEST, CLIENT_STATIC_FILES_PATH, CLIENT_STATIC_FILES_RUNTIME_MAIN, CLIENT_STATIC_FILES_RUNTIME_MAIN_APP, CLIENT_STATIC_FILES_RUNTIME_POLYFILLS_SYMBOL, CLIENT_STATIC_FILES_RUNTIME_REACT_REFRESH, CLIENT_STATIC_FILES_RUNTIME_AMP, SYSTEM_ENTRYPOINTS } from '../../../shared/lib/constants';
import getRouteFromEntrypoint from '../../../server/get-route-from-entrypoint';
import { ampFirstEntryNamesMap } from './next-drop-client-page-plugin';
import { getSortedRoutes } from '../../../shared/lib/router/utils';
import { spans } from './profiling-plugin';
import { Span } from '../../../trace';
// Add the runtime ssg manifest file as a lazy-loaded file dependency.
// We also stub this file out for development mode (when it is not
// generated).
export const srcEmptySsgManifest = `self.__SSG_MANIFEST=new Set;self.__SSG_MANIFEST_CB&&self.__SSG_MANIFEST_CB()`;
// nodejs: '/static/<build id>/low-priority.js'
function buildNodejsLowPriorityPath(filename, buildId) {
    return `${CLIENT_STATIC_FILES_PATH}/${buildId}/${filename}`;
}
function createEdgeRuntimeManifest(originAssetMap) {
    const manifestFilenames = [
        '_buildManifest.js',
        '_ssgManifest.js'
    ];
    const assetMap = {
        ...originAssetMap,
        lowPriorityFiles: []
    };
    const manifestDefCode = `self.__BUILD_MANIFEST = ${JSON.stringify(assetMap, null, 2)};\n`;
    // edge lowPriorityFiles item: '"/static/" + process.env.__NEXT_BUILD_ID + "/low-priority.js"'.
    // Since lowPriorityFiles is not fixed and relying on `process.env.__NEXT_BUILD_ID`, we'll produce code creating it dynamically.
    const lowPriorityFilesCode = `self.__BUILD_MANIFEST.lowPriorityFiles = [\n` + manifestFilenames.map((filename)=>{
        return `"/static/" + process.env.__NEXT_BUILD_ID + "/${filename}",\n`;
    }).join(',') + `\n];`;
    return manifestDefCode + lowPriorityFilesCode;
}
function normalizeRewrite(item) {
    return {
        has: item.has,
        source: item.source,
        destination: item.destination
    };
}
export function normalizeRewritesForBuildManifest(rewrites) {
    var _rewrites_afterFiles_map, _rewrites_afterFiles, _rewrites_beforeFiles_map, _rewrites_beforeFiles, _rewrites_fallback_map, _rewrites_fallback;
    return {
        afterFiles: (_rewrites_afterFiles = rewrites.afterFiles) == null ? void 0 : (_rewrites_afterFiles_map = _rewrites_afterFiles.map(processRoute)) == null ? void 0 : _rewrites_afterFiles_map.map((item)=>normalizeRewrite(item)),
        beforeFiles: (_rewrites_beforeFiles = rewrites.beforeFiles) == null ? void 0 : (_rewrites_beforeFiles_map = _rewrites_beforeFiles.map(processRoute)) == null ? void 0 : _rewrites_beforeFiles_map.map((item)=>normalizeRewrite(item)),
        fallback: (_rewrites_fallback = rewrites.fallback) == null ? void 0 : (_rewrites_fallback_map = _rewrites_fallback.map(processRoute)) == null ? void 0 : _rewrites_fallback_map.map((item)=>normalizeRewrite(item))
    };
}
// This function takes the asset map generated in BuildManifestPlugin and creates a
// reduced version to send to the client.
export function generateClientManifest(assetMap, rewrites, clientRouterFilters, compiler, compilation) {
    const compilationSpan = compilation ? spans.get(compilation) : compiler ? spans.get(compiler) : new Span({
        name: 'client-manifest'
    });
    const genClientManifestSpan = compilationSpan == null ? void 0 : compilationSpan.traceChild('NextJsBuildManifest-generateClientManifest');
    return genClientManifestSpan == null ? void 0 : genClientManifestSpan.traceFn(()=>{
        const clientManifest = {
            __rewrites: normalizeRewritesForBuildManifest(rewrites),
            __routerFilterStatic: clientRouterFilters == null ? void 0 : clientRouterFilters.staticFilter,
            __routerFilterDynamic: clientRouterFilters == null ? void 0 : clientRouterFilters.dynamicFilter
        };
        const appDependencies = new Set(assetMap.pages['/_app']);
        const sortedPageKeys = getSortedRoutes(Object.keys(assetMap.pages));
        sortedPageKeys.forEach((page)=>{
            const dependencies = assetMap.pages[page];
            if (page === '/_app') return;
            // Filter out dependencies in the _app entry, because those will have already
            // been loaded by the client prior to a navigation event
            const filteredDeps = dependencies.filter((dep)=>!appDependencies.has(dep));
            // The manifest can omit the page if it has no requirements
            if (filteredDeps.length) {
                clientManifest[page] = filteredDeps;
            }
        });
        // provide the sorted pages as an array so we don't rely on the object's keys
        // being in order and we don't slow down look-up time for page assets
        clientManifest.sortedPages = sortedPageKeys;
        return devalue(clientManifest);
    });
}
export function getEntrypointFiles(entrypoint) {
    return (entrypoint == null ? void 0 : entrypoint.getFiles().filter((file)=>{
        // We don't want to include `.hot-update.js` files into the initial page
        return /(?<!\.hot-update)\.(js|css)($|\?)/.test(file);
    }).map((file)=>file.replace(/\\/g, '/'))) ?? [];
}
export const processRoute = (r)=>{
    var _rewrite_destination;
    const rewrite = {
        ...r
    };
    // omit external rewrite destinations since these aren't
    // handled client-side
    if (!(rewrite == null ? void 0 : (_rewrite_destination = rewrite.destination) == null ? void 0 : _rewrite_destination.startsWith('/'))) {
        delete rewrite.destination;
    }
    return rewrite;
};
// This plugin creates a build-manifest.json for all assets that are being output
// It has a mapping of "entry" filename to real filename. Because the real filename can be hashed in production
export default class BuildManifestPlugin {
    constructor(options){
        this.buildId = options.buildId;
        this.isDevFallback = !!options.isDevFallback;
        this.rewrites = {
            beforeFiles: [],
            afterFiles: [],
            fallback: []
        };
        this.appDirEnabled = options.appDirEnabled;
        this.clientRouterFilters = options.clientRouterFilters;
        this.rewrites.beforeFiles = options.rewrites.beforeFiles.map(processRoute);
        this.rewrites.afterFiles = options.rewrites.afterFiles.map(processRoute);
        this.rewrites.fallback = options.rewrites.fallback.map(processRoute);
    }
    createAssets(compiler, compilation, assets) {
        const compilationSpan = spans.get(compilation) || spans.get(compiler);
        const createAssetsSpan = compilationSpan == null ? void 0 : compilationSpan.traceChild('NextJsBuildManifest-createassets');
        return createAssetsSpan == null ? void 0 : createAssetsSpan.traceFn(()=>{
            const entrypoints = compilation.entrypoints;
            const assetMap = {
                polyfillFiles: [],
                devFiles: [],
                ampDevFiles: [],
                lowPriorityFiles: [],
                rootMainFiles: [],
                rootMainFilesTree: {},
                pages: {
                    '/_app': []
                },
                ampFirstPages: []
            };
            const ampFirstEntryNames = ampFirstEntryNamesMap.get(compilation);
            if (ampFirstEntryNames) {
                for (const entryName of ampFirstEntryNames){
                    const pagePath = getRouteFromEntrypoint(entryName);
                    if (!pagePath) {
                        continue;
                    }
                    assetMap.ampFirstPages.push(pagePath);
                }
            }
            const mainFiles = new Set(getEntrypointFiles(entrypoints.get(CLIENT_STATIC_FILES_RUNTIME_MAIN)));
            if (this.appDirEnabled) {
                assetMap.rootMainFiles = [
                    ...new Set(getEntrypointFiles(entrypoints.get(CLIENT_STATIC_FILES_RUNTIME_MAIN_APP)))
                ];
            }
            const compilationAssets = compilation.getAssets();
            assetMap.polyfillFiles = compilationAssets.filter((p)=>{
                // Ensure only .js files are passed through
                if (!p.name.endsWith('.js')) {
                    return false;
                }
                return p.info && CLIENT_STATIC_FILES_RUNTIME_POLYFILLS_SYMBOL in p.info;
            }).map((v)=>v.name);
            assetMap.devFiles = getEntrypointFiles(entrypoints.get(CLIENT_STATIC_FILES_RUNTIME_REACT_REFRESH)).filter((file)=>!mainFiles.has(file));
            assetMap.ampDevFiles = getEntrypointFiles(entrypoints.get(CLIENT_STATIC_FILES_RUNTIME_AMP));
            for (const entrypoint of compilation.entrypoints.values()){
                if (SYSTEM_ENTRYPOINTS.has(entrypoint.name)) continue;
                const pagePath = getRouteFromEntrypoint(entrypoint.name);
                if (!pagePath) {
                    continue;
                }
                const filesForPage = getEntrypointFiles(entrypoint);
                assetMap.pages[pagePath] = [
                    ...new Set([
                        ...mainFiles,
                        ...filesForPage
                    ])
                ];
            }
            if (!this.isDevFallback) {
                // Add the runtime build manifest file (generated later in this file)
                // as a dependency for the app. If the flag is false, the file won't be
                // downloaded by the client.
                const buildManifestPath = buildNodejsLowPriorityPath('_buildManifest.js', this.buildId);
                const ssgManifestPath = buildNodejsLowPriorityPath('_ssgManifest.js', this.buildId);
                assetMap.lowPriorityFiles.push(buildManifestPath, ssgManifestPath);
                assets[ssgManifestPath] = new sources.RawSource(srcEmptySsgManifest);
            }
            assetMap.pages = Object.keys(assetMap.pages).sort().reduce(// eslint-disable-next-line
            (a, c)=>(a[c] = assetMap.pages[c], a), {});
            let buildManifestName = BUILD_MANIFEST;
            if (this.isDevFallback) {
                buildManifestName = `fallback-${BUILD_MANIFEST}`;
            }
            assets[buildManifestName] = new sources.RawSource(JSON.stringify(assetMap, null, 2));
            assets[`server/${MIDDLEWARE_BUILD_MANIFEST}.js`] = new sources.RawSource(`${createEdgeRuntimeManifest(assetMap)}`);
            if (!this.isDevFallback) {
                const clientManifestPath = `${CLIENT_STATIC_FILES_PATH}/${this.buildId}/_buildManifest.js`;
                assets[clientManifestPath] = new sources.RawSource(`self.__BUILD_MANIFEST = ${generateClientManifest(assetMap, this.rewrites, this.clientRouterFilters, compiler, compilation)};self.__BUILD_MANIFEST_CB && self.__BUILD_MANIFEST_CB()`);
            }
            return assets;
        });
    }
    apply(compiler) {
        compiler.hooks.make.tap('NextJsBuildManifest', (compilation)=>{
            compilation.hooks.processAssets.tap({
                name: 'NextJsBuildManifest',
                stage: webpack.Compilation.PROCESS_ASSETS_STAGE_ADDITIONS
            }, (assets)=>{
                this.createAssets(compiler, compilation, assets);
            });
        });
        return;
    }
}

//# sourceMappingURL=build-manifest-plugin.js.map