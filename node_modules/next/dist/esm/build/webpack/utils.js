import { isMetadataRoute } from '../../lib/metadata/is-metadata-route';
export function traverseModules(compilation, callback, filterChunkGroup) {
    compilation.chunkGroups.forEach((chunkGroup)=>{
        if (filterChunkGroup && !filterChunkGroup(chunkGroup)) {
            return;
        }
        chunkGroup.chunks.forEach((chunk)=>{
            const chunkModules = compilation.chunkGraph.getChunkModulesIterable(chunk);
            for (const mod of chunkModules){
                var _compilation_chunkGraph_getModuleId;
                const modId = (_compilation_chunkGraph_getModuleId = compilation.chunkGraph.getModuleId(mod)) == null ? void 0 : _compilation_chunkGraph_getModuleId.toString();
                if (modId) callback(mod, chunk, chunkGroup, modId);
                const anyModule = mod;
                if (anyModule.modules) {
                    for (const subMod of anyModule.modules)if (modId) callback(subMod, chunk, chunkGroup, modId);
                }
            }
        });
    });
}
// Loop over all the entry modules.
export function forEachEntryModule(compilation, callback) {
    for (const [name, entry] of compilation.entries.entries()){
        var _entry_dependencies;
        // Skip for entries under pages/
        if (name.startsWith('pages/') || // Skip for metadata route handlers
        name.startsWith('app/') && isMetadataRoute(name)) {
            continue;
        }
        // Check if the page entry is a server component or not.
        const entryDependency = (_entry_dependencies = entry.dependencies) == null ? void 0 : _entry_dependencies[0];
        // Ensure only next-app-loader entries are handled.
        if (!entryDependency || !entryDependency.request) continue;
        const request = entryDependency.request;
        if (!request.startsWith('next-edge-ssr-loader?') && !request.startsWith('next-edge-app-route-loader?') && !request.startsWith('next-app-loader?')) continue;
        let entryModule = compilation.moduleGraph.getResolvedModule(entryDependency);
        if (request.startsWith('next-edge-ssr-loader?') || request.startsWith('next-edge-app-route-loader?')) {
            entryModule.dependencies.forEach((dependency)=>{
                const modRequest = dependency.request;
                if (modRequest == null ? void 0 : modRequest.includes('next-app-loader')) {
                    entryModule = compilation.moduleGraph.getResolvedModule(dependency);
                }
            });
        }
        callback({
            name,
            entryModule
        });
    }
}
export function formatBarrelOptimizedResource(resource, matchResource) {
    return `${resource}@${matchResource}`;
}
export function getModuleReferencesInOrder(module, moduleGraph) {
    const connections = [];
    for (const connection of moduleGraph.getOutgoingConnections(module)){
        if (connection.dependency && connection.module) {
            connections.push({
                connection,
                index: moduleGraph.getParentBlockIndex(connection.dependency)
            });
        }
    }
    connections.sort((a, b)=>a.index - b.index);
    return connections.map((c)=>c.connection);
}

//# sourceMappingURL=utils.js.map