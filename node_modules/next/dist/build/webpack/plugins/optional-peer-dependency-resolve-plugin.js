"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "OptionalPeerDependencyResolverPlugin", {
    enumerable: true,
    get: function() {
        return OptionalPeerDependencyResolverPlugin;
    }
});
const pluginSymbol = Symbol("OptionalPeerDependencyResolverPlugin");
class OptionalPeerDependencyResolverPlugin {
    apply(resolver) {
        const target = resolver.ensureHook("raw-module");
        target.tapAsync("OptionalPeerDependencyResolverPlugin", (request, resolveContext, callback)=>{
            var // popping the stack to prevent the recursion check
            _resolveContext_stack;
            // if we've already recursed into this plugin, we want to skip it
            if (request[pluginSymbol]) {
                return callback();
            }
            (_resolveContext_stack = resolveContext.stack) == null ? void 0 : _resolveContext_stack.delete(Array.from(resolveContext.stack).pop());
            resolver.doResolve(target, // when we call doResolve again, we need to make sure we don't
            // recurse into this plugin again
            {
                ...request,
                [pluginSymbol]: true
            }, null, resolveContext, (err, result)=>{
                var _request_descriptionFileData;
                if (!result && (request == null ? void 0 : (_request_descriptionFileData = request.descriptionFileData) == null ? void 0 : _request_descriptionFileData.peerDependenciesMeta) && request.request) {
                    const peerDependenciesMeta = request.descriptionFileData.peerDependenciesMeta;
                    const isOptional = peerDependenciesMeta && peerDependenciesMeta[request.request] && peerDependenciesMeta[request.request].optional;
                    if (isOptional) {
                        return callback(null, {
                            path: false
                        });
                    }
                }
                return callback(err, result);
            });
        });
    }
}

//# sourceMappingURL=optional-peer-dependency-resolve-plugin.js.map