/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */ "use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "ClientReferenceManifestPlugin", {
    enumerable: true,
    get: function() {
        return ClientReferenceManifestPlugin;
    }
});
const _path = /*#__PURE__*/ _interop_require_wildcard(require("path"));
const _webpack = require("next/dist/compiled/webpack/webpack");
const _constants = require("../../../shared/lib/constants");
const _buildcontext = require("../../build-context");
const _constants1 = require("../../../lib/constants");
const _normalizepagepath = require("../../../shared/lib/page-path/normalize-page-path");
const _deploymentid = require("../../deployment-id");
const _utils = require("../utils");
const _encodeuripath = require("../../../shared/lib/encode-uri-path");
const _ismetadataroute = require("../../../lib/metadata/is-metadata-route");
function _getRequireWildcardCache(nodeInterop) {
    if (typeof WeakMap !== "function") return null;
    var cacheBabelInterop = new WeakMap();
    var cacheNodeInterop = new WeakMap();
    return (_getRequireWildcardCache = function(nodeInterop) {
        return nodeInterop ? cacheNodeInterop : cacheBabelInterop;
    })(nodeInterop);
}
function _interop_require_wildcard(obj, nodeInterop) {
    if (!nodeInterop && obj && obj.__esModule) {
        return obj;
    }
    if (obj === null || typeof obj !== "object" && typeof obj !== "function") {
        return {
            default: obj
        };
    }
    var cache = _getRequireWildcardCache(nodeInterop);
    if (cache && cache.has(obj)) {
        return cache.get(obj);
    }
    var newObj = {
        __proto__: null
    };
    var hasPropertyDescriptor = Object.defineProperty && Object.getOwnPropertyDescriptor;
    for(var key in obj){
        if (key !== "default" && Object.prototype.hasOwnProperty.call(obj, key)) {
            var desc = hasPropertyDescriptor ? Object.getOwnPropertyDescriptor(obj, key) : null;
            if (desc && (desc.get || desc.set)) {
                Object.defineProperty(newObj, key, desc);
            } else {
                newObj[key] = obj[key];
            }
        }
    }
    newObj.default = obj;
    if (cache) {
        cache.set(obj, newObj);
    }
    return newObj;
}
const pluginState = (0, _buildcontext.getProxiedPluginState)({
    ssrModules: {},
    edgeSsrModules: {},
    rscModules: {},
    edgeRscModules: {}
});
function getAppPathRequiredChunks(chunkGroup, excludedFiles) {
    const deploymentIdChunkQuery = (0, _deploymentid.getDeploymentIdQueryOrEmptyString)();
    const chunks = [];
    chunkGroup.chunks.forEach((chunk)=>{
        if (_constants.SYSTEM_ENTRYPOINTS.has(chunk.name || '')) {
            return null;
        }
        // Get the actual chunk file names from the chunk file list.
        // It's possible that the chunk is generated via `import()`, in
        // that case the chunk file name will be '[name].[contenthash]'
        // instead of '[name]-[chunkhash]'.
        if (chunk.id != null) {
            const chunkId = '' + chunk.id;
            chunk.files.forEach((file)=>{
                // It's possible that a chunk also emits CSS files, that will
                // be handled separatedly.
                if (!file.endsWith('.js')) return null;
                if (file.endsWith('.hot-update.js')) return null;
                if (excludedFiles.has(file)) return null;
                // We encode the file as a URI because our server (and many other services such as S3)
                // expect to receive reserved characters such as `[` and `]` as encoded. This was
                // previously done for dynamic chunks by patching the webpack runtime but we want
                // these filenames to be managed by React's Flight runtime instead and so we need
                // to implement any special handling of the file name here.
                return chunks.push(chunkId, (0, _encodeuripath.encodeURIPath)(file) + deploymentIdChunkQuery);
            });
        }
    });
    return chunks;
}
// Normalize the entry names to their "group names" so a page can easily track
// all the manifest items it needs from parent groups by looking up the group
// segments:
// - app/foo/loading -> app/foo
// - app/foo/page -> app/foo
// - app/(group)/@named/foo/page -> app/foo
// - app/(.)foo/(..)bar/loading -> app/bar
// - app/[...catchAll]/page -> app
// - app/foo/@slot/[...catchAll]/page -> app/foo
function entryNameToGroupName(entryName) {
    let groupName = entryName.slice(0, entryName.lastIndexOf('/'))// Remove slots
    .replace(/\/@[^/]+/g, '')// Remove the group with lookahead to make sure it's not interception route
    .replace(/\/\([^/]+\)(?=(\/|$))/g, '')// Remove catch-all routes since they should be part of the parent group that the catch-all would apply to.
    // This is necessary to support parallel routes since multiple page components can be rendered on the same page.
    // In order to do that, we need to ensure that the manifests are merged together by putting them in the same group.
    .replace(/\/\[?\[\.\.\.[^\]]*]]?/g, '');
    // Interception routes
    groupName = groupName.replace(/^.+\/\(\.\.\.\)/g, 'app/').replace(/\/\(\.\)/g, '/');
    // Interception routes (recursive)
    while(/\/[^/]+\/\(\.\.\)/.test(groupName)){
        groupName = groupName.replace(/\/[^/]+\/\(\.\.\)/g, '/');
    }
    return groupName;
}
function mergeManifest(manifest, manifestToMerge) {
    Object.assign(manifest.clientModules, manifestToMerge.clientModules);
    Object.assign(manifest.ssrModuleMapping, manifestToMerge.ssrModuleMapping);
    Object.assign(manifest.edgeSSRModuleMapping, manifestToMerge.edgeSSRModuleMapping);
    Object.assign(manifest.entryCSSFiles, manifestToMerge.entryCSSFiles);
    Object.assign(manifest.rscModuleMapping, manifestToMerge.rscModuleMapping);
    Object.assign(manifest.edgeRscModuleMapping, manifestToMerge.edgeRscModuleMapping);
}
const PLUGIN_NAME = 'ClientReferenceManifestPlugin';
class ClientReferenceManifestPlugin {
    constructor(options){
        this.dev = false;
        this.dev = options.dev;
        this.appDir = options.appDir;
        this.appDirBase = _path.default.dirname(this.appDir) + _path.default.sep;
        this.experimentalInlineCss = options.experimentalInlineCss;
    }
    apply(compiler) {
        compiler.hooks.compilation.tap(PLUGIN_NAME, (compilation, { normalModuleFactory })=>{
            compilation.dependencyFactories.set(_webpack.webpack.dependencies.ModuleDependency, normalModuleFactory);
            compilation.dependencyTemplates.set(_webpack.webpack.dependencies.ModuleDependency, new _webpack.webpack.dependencies.NullDependency.Template());
            compilation.hooks.processAssets.tap({
                name: PLUGIN_NAME,
                // Have to be in the optimize stage to run after updating the CSS
                // asset hash via extract mini css plugin.
                stage: _webpack.webpack.Compilation.PROCESS_ASSETS_STAGE_OPTIMIZE_HASH
            }, (assets)=>this.createAsset(assets, compilation, compiler.context));
        });
    }
    createAsset(assets, compilation, context) {
        var _compilation_entrypoints_get;
        const manifestsPerGroup = new Map();
        const manifestEntryFiles = [];
        const configuredCrossOriginLoading = compilation.outputOptions.crossOriginLoading;
        const crossOriginMode = typeof configuredCrossOriginLoading === 'string' ? configuredCrossOriginLoading === 'use-credentials' ? configuredCrossOriginLoading : 'anonymous' : null;
        if (typeof compilation.outputOptions.publicPath !== 'string') {
            throw new Error('Expected webpack publicPath to be a string when using App Router. To customize where static assets are loaded from, use the `assetPrefix` option in next.config.js. If you are customizing your webpack config please make sure you are not modifying or removing the publicPath configuration option');
        }
        const prefix = compilation.outputOptions.publicPath || '';
        // We want to omit any files that will always be loaded on any App Router page
        // because they will already be loaded by the main entrypoint.
        const rootMainFiles = new Set();
        (_compilation_entrypoints_get = compilation.entrypoints.get(_constants.CLIENT_STATIC_FILES_RUNTIME_MAIN_APP)) == null ? void 0 : _compilation_entrypoints_get.getFiles().forEach((file)=>{
            if (/(?<!\.hot-update)\.(js|css)($|\?)/.test(file)) {
                rootMainFiles.add(file.replace(/\\/g, '/'));
            }
        });
        for (let [entryName, entrypoint] of compilation.entrypoints){
            if (entryName === _constants.CLIENT_STATIC_FILES_RUNTIME_MAIN_APP || entryName === _constants.APP_CLIENT_INTERNALS) {
                entryName = '';
            } else if (!/^app[\\/]/.test(entryName)) {
                continue;
            }
            const manifest = {
                moduleLoading: {
                    prefix,
                    crossOrigin: crossOriginMode
                },
                ssrModuleMapping: {},
                edgeSSRModuleMapping: {},
                clientModules: {},
                entryCSSFiles: {},
                rscModuleMapping: {},
                edgeRscModuleMapping: {}
            };
            // Absolute path without the extension
            const chunkEntryName = (this.appDirBase + entryName).replace(/[\\/]/g, _path.default.sep);
            manifest.entryCSSFiles[chunkEntryName] = entrypoint.getFiles().filter((f)=>!f.startsWith('static/css/pages/') && f.endsWith('.css')).map((file)=>{
                const source = compilation.assets[file].source();
                if (this.experimentalInlineCss && // Inline CSS currently does not work properly with HMR, so we only
                // inline CSS in production.
                !this.dev) {
                    return {
                        inlined: true,
                        path: file,
                        content: typeof source === 'string' ? source : source.toString()
                    };
                }
                return {
                    inlined: false,
                    path: file
                };
            });
            const requiredChunks = getAppPathRequiredChunks(entrypoint, rootMainFiles);
            const recordModule = (modId, mod)=>{
                var _mod_resourceResolveData, _mod_resourceResolveData1, _mod_matchResource;
                let resource = mod.type === 'css/mini-extract' ? mod._identifier.slice(mod._identifier.lastIndexOf('!') + 1) : mod.resource;
                if (!resource) {
                    return;
                }
                const moduleReferences = manifest.clientModules;
                const moduleIdMapping = manifest.ssrModuleMapping;
                const edgeModuleIdMapping = manifest.edgeSSRModuleMapping;
                const rscIdMapping = manifest.rscModuleMapping;
                const edgeRscIdMapping = manifest.edgeRscModuleMapping;
                // Note that this isn't that reliable as webpack is still possible to assign
                // additional queries to make sure there's no conflict even using the `named`
                // module ID strategy.
                let ssrNamedModuleId = (0, _path.relative)(context, ((_mod_resourceResolveData = mod.resourceResolveData) == null ? void 0 : _mod_resourceResolveData.path) || resource);
                const rscNamedModuleId = (0, _path.relative)(context, ((_mod_resourceResolveData1 = mod.resourceResolveData) == null ? void 0 : _mod_resourceResolveData1.path) || resource);
                if (!ssrNamedModuleId.startsWith('.')) ssrNamedModuleId = `./${ssrNamedModuleId.replace(/\\/g, '/')}`;
                // The client compiler will always use the CJS Next.js build, so here we
                // also add the mapping for the ESM build (Edge runtime) to consume.
                const esmResource = /[\\/]next[\\/]dist[\\/]/.test(resource) ? resource.replace(/[\\/]next[\\/]dist[\\/]/, '/next/dist/esm/'.replace(/\//g, _path.default.sep)) : null;
                // An extra query param is added to the resource key when it's optimized
                // through the Barrel Loader. That's because the same file might be created
                // as multiple modules (depending on what you import from it).
                // See also: webpack/loaders/next-flight-loader/index.ts.
                if ((_mod_matchResource = mod.matchResource) == null ? void 0 : _mod_matchResource.startsWith(_constants.BARREL_OPTIMIZATION_PREFIX)) {
                    ssrNamedModuleId = (0, _utils.formatBarrelOptimizedResource)(ssrNamedModuleId, mod.matchResource);
                    resource = (0, _utils.formatBarrelOptimizedResource)(resource, mod.matchResource);
                }
                function addClientReference() {
                    var _pluginState_ssrModules_ssrNamedModuleId, _pluginState_edgeSsrModules_ssrNamedModuleId;
                    const isAsync = Boolean(compilation.moduleGraph.isAsync(mod) || ((_pluginState_ssrModules_ssrNamedModuleId = pluginState.ssrModules[ssrNamedModuleId]) == null ? void 0 : _pluginState_ssrModules_ssrNamedModuleId.async) || ((_pluginState_edgeSsrModules_ssrNamedModuleId = pluginState.edgeSsrModules[ssrNamedModuleId]) == null ? void 0 : _pluginState_edgeSsrModules_ssrNamedModuleId.async));
                    const exportName = resource;
                    manifest.clientModules[exportName] = {
                        id: modId,
                        name: '*',
                        chunks: requiredChunks,
                        async: isAsync
                    };
                    if (esmResource) {
                        const edgeExportName = esmResource;
                        manifest.clientModules[edgeExportName] = manifest.clientModules[exportName];
                    }
                }
                function addSSRIdMapping() {
                    const exportName = resource;
                    const moduleInfo = pluginState.ssrModules[ssrNamedModuleId];
                    if (moduleInfo) {
                        moduleIdMapping[modId] = moduleIdMapping[modId] || {};
                        moduleIdMapping[modId]['*'] = {
                            ...manifest.clientModules[exportName],
                            // During SSR, we don't have external chunks to load on the server
                            // side with our architecture of Webpack / Turbopack. We can keep
                            // this field empty to save some bytes.
                            chunks: [],
                            id: moduleInfo.moduleId,
                            async: moduleInfo.async
                        };
                    }
                    const edgeModuleInfo = pluginState.edgeSsrModules[ssrNamedModuleId];
                    if (edgeModuleInfo) {
                        edgeModuleIdMapping[modId] = edgeModuleIdMapping[modId] || {};
                        edgeModuleIdMapping[modId]['*'] = {
                            ...manifest.clientModules[exportName],
                            // During SSR, we don't have external chunks to load on the server
                            // side with our architecture of Webpack / Turbopack. We can keep
                            // this field empty to save some bytes.
                            chunks: [],
                            id: edgeModuleInfo.moduleId,
                            async: edgeModuleInfo.async
                        };
                    }
                }
                function addRSCIdMapping() {
                    const exportName = resource;
                    const moduleInfo = pluginState.rscModules[rscNamedModuleId];
                    if (moduleInfo) {
                        rscIdMapping[modId] = rscIdMapping[modId] || {};
                        rscIdMapping[modId]['*'] = {
                            ...manifest.clientModules[exportName],
                            // During SSR, we don't have external chunks to load on the server
                            // side with our architecture of Webpack / Turbopack. We can keep
                            // this field empty to save some bytes.
                            chunks: [],
                            id: moduleInfo.moduleId,
                            async: moduleInfo.async
                        };
                    }
                    const edgeModuleInfo = pluginState.ssrModules[rscNamedModuleId];
                    if (edgeModuleInfo) {
                        edgeRscIdMapping[modId] = edgeRscIdMapping[modId] || {};
                        edgeRscIdMapping[modId]['*'] = {
                            ...manifest.clientModules[exportName],
                            // During SSR, we don't have external chunks to load on the server
                            // side with our architecture of Webpack / Turbopack. We can keep
                            // this field empty to save some bytes.
                            chunks: [],
                            id: edgeModuleInfo.moduleId,
                            async: edgeModuleInfo.async
                        };
                    }
                }
                addClientReference();
                addSSRIdMapping();
                addRSCIdMapping();
                manifest.clientModules = moduleReferences;
                manifest.ssrModuleMapping = moduleIdMapping;
                manifest.edgeSSRModuleMapping = edgeModuleIdMapping;
                manifest.rscModuleMapping = rscIdMapping;
                manifest.edgeRscModuleMapping = edgeRscIdMapping;
            };
            const checkedChunkGroups = new Set();
            const checkedChunks = new Set();
            function recordChunkGroup(chunkGroup) {
                // Ensure recursion is stopped if we've already checked this chunk group.
                if (checkedChunkGroups.has(chunkGroup)) return;
                checkedChunkGroups.add(chunkGroup);
                // Only apply following logic to client module requests from client entry,
                // or if the module is marked as client module. That's because other
                // client modules don't need to be in the manifest at all as they're
                // never be referenced by the server/client boundary.
                // This saves a lot of bytes in the manifest.
                chunkGroup.chunks.forEach((chunk)=>{
                    // Ensure recursion is stopped if we've already checked this chunk.
                    if (checkedChunks.has(chunk)) return;
                    checkedChunks.add(chunk);
                    const entryMods = compilation.chunkGraph.getChunkEntryModulesIterable(chunk);
                    for (const mod of entryMods){
                        if (mod.layer !== _constants1.WEBPACK_LAYERS.appPagesBrowser) continue;
                        const request = mod.request;
                        if (!request || !request.includes('next-flight-client-entry-loader.js?')) {
                            continue;
                        }
                        const connections = (0, _utils.getModuleReferencesInOrder)(mod, compilation.moduleGraph);
                        for (const connection of connections){
                            const dependency = connection.dependency;
                            if (!dependency) continue;
                            const clientEntryMod = compilation.moduleGraph.getResolvedModule(dependency);
                            const modId = compilation.chunkGraph.getModuleId(clientEntryMod);
                            if (modId !== null) {
                                recordModule(modId, clientEntryMod);
                            } else {
                                var _connection_module;
                                // If this is a concatenation, register each child to the parent ID.
                                if (((_connection_module = connection.module) == null ? void 0 : _connection_module.constructor.name) === 'ConcatenatedModule') {
                                    const concatenatedMod = connection.module;
                                    const concatenatedModId = compilation.chunkGraph.getModuleId(concatenatedMod);
                                    if (concatenatedModId) {
                                        recordModule(concatenatedModId, clientEntryMod);
                                    }
                                }
                            }
                        }
                    }
                });
                // Walk through all children chunk groups too.
                for (const child of chunkGroup.childrenIterable){
                    recordChunkGroup(child);
                }
            }
            recordChunkGroup(entrypoint);
            // A page's entry name can have extensions. For example, these are both valid:
            // - app/foo/page
            // - app/foo/page.page
            if (/\/page(\.[^/]+)?$/.test(entryName)) {
                manifestEntryFiles.push(entryName.replace(/\/page(\.[^/]+)?$/, '/page'));
            }
            // We also need to create manifests for route handler entrypoints
            // (excluding metadata route handlers) to enable `'use cache'`.
            if (/\/route$/.test(entryName) && !(0, _ismetadataroute.isMetadataRoute)(entryName)) {
                manifestEntryFiles.push(entryName);
            }
            const groupName = entryNameToGroupName(entryName);
            if (!manifestsPerGroup.has(groupName)) {
                manifestsPerGroup.set(groupName, []);
            }
            manifestsPerGroup.get(groupName).push(manifest);
        }
        // Generate per-page manifests.
        for (const pageName of manifestEntryFiles){
            const mergedManifest = {
                moduleLoading: {
                    prefix,
                    crossOrigin: crossOriginMode
                },
                ssrModuleMapping: {},
                edgeSSRModuleMapping: {},
                clientModules: {},
                entryCSSFiles: {},
                rscModuleMapping: {},
                edgeRscModuleMapping: {}
            };
            const segments = [
                ...entryNameToGroupName(pageName).split('/'),
                'page'
            ];
            let group = '';
            for (const segment of segments){
                for (const manifest of manifestsPerGroup.get(group) || []){
                    mergeManifest(mergedManifest, manifest);
                }
                group += (group ? '/' : '') + segment;
            }
            const json = JSON.stringify(mergedManifest);
            const pagePath = pageName.replace(/%5F/g, '_');
            const pageBundlePath = (0, _normalizepagepath.normalizePagePath)(pagePath.slice('app'.length));
            assets['server/app' + pageBundlePath + '_' + _constants.CLIENT_REFERENCE_MANIFEST + '.js'] = new _webpack.sources.RawSource(`globalThis.__RSC_MANIFEST=(globalThis.__RSC_MANIFEST||{});globalThis.__RSC_MANIFEST[${JSON.stringify(pagePath.slice('app'.length))}]=${json}`);
        }
    }
}

//# sourceMappingURL=flight-manifest-plugin.js.map