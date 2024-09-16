"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    forEachEntryModule: null,
    formatBarrelOptimizedResource: null,
    getModuleReferencesInOrder: null,
    traverseModules: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    forEachEntryModule: function() {
        return forEachEntryModule;
    },
    formatBarrelOptimizedResource: function() {
        return formatBarrelOptimizedResource;
    },
    getModuleReferencesInOrder: function() {
        return getModuleReferencesInOrder;
    },
    traverseModules: function() {
        return traverseModules;
    }
});
const _isapprouteroute = require("../../lib/is-app-route-route");
function traverseModules(compilation, callback, filterChunkGroup) {
    compilation.chunkGroups.forEach((chunkGroup)=>{
        if (filterChunkGroup && !filterChunkGroup(chunkGroup)) {
            return;
        }
        chunkGroup.chunks.forEach((chunk)=>{
            const chunkModules = compilation.chunkGraph.getChunkModulesIterable(chunk);
            for (const mod of chunkModules){
                var _compilation_chunkGraph_getModuleId;
                const modId = (_compilation_chunkGraph_getModuleId = compilation.chunkGraph.getModuleId(mod)) == null ? void 0 : _compilation_chunkGraph_getModuleId.toString();
                callback(mod, chunk, chunkGroup, modId);
                const anyModule = mod;
                if (anyModule.modules) {
                    for (const subMod of anyModule.modules)callback(subMod, chunk, chunkGroup, modId);
                }
            }
        });
    });
}
function forEachEntryModule(compilation, callback) {
    for (const [name, entry] of compilation.entries.entries()){
        var _entry_dependencies;
        // Skip for entries under pages/
        if (name.startsWith("pages/") || // Skip for route.js entries
        name.startsWith("app/") && (0, _isapprouteroute.isAppRouteRoute)(name)) {
            continue;
        }
        // Check if the page entry is a server component or not.
        const entryDependency = (_entry_dependencies = entry.dependencies) == null ? void 0 : _entry_dependencies[0];
        // Ensure only next-app-loader entries are handled.
        if (!entryDependency || !entryDependency.request) continue;
        const request = entryDependency.request;
        if (!request.startsWith("next-edge-ssr-loader?") && !request.startsWith("next-app-loader?")) continue;
        let entryModule = compilation.moduleGraph.getResolvedModule(entryDependency);
        if (request.startsWith("next-edge-ssr-loader?")) {
            entryModule.dependencies.forEach((dependency)=>{
                const modRequest = dependency.request;
                if (modRequest == null ? void 0 : modRequest.includes("next-app-loader")) {
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
function formatBarrelOptimizedResource(resource, matchResource) {
    return `${resource}@${matchResource}`;
}
function getModuleReferencesInOrder(module1, moduleGraph) {
    const connections = [];
    for (const connection of moduleGraph.getOutgoingConnections(module1)){
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