"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    clearManifestCache: null,
    evalManifest: null,
    loadManifest: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    clearManifestCache: function() {
        return clearManifestCache;
    },
    evalManifest: function() {
        return evalManifest;
    },
    loadManifest: function() {
        return loadManifest;
    }
});
const _fs = require("fs");
const _vm = require("vm");
const _deepfreeze = require("../shared/lib/deep-freeze");
const sharedCache = new Map();
function loadManifest(path, shouldCache = true, cache = sharedCache) {
    const cached = shouldCache && cache.get(path);
    if (cached) {
        return cached;
    }
    let manifest = JSON.parse((0, _fs.readFileSync)(path, "utf8"));
    // Freeze the manifest so it cannot be modified if we're caching it.
    if (shouldCache) {
        manifest = (0, _deepfreeze.deepFreeze)(manifest);
    }
    if (shouldCache) {
        cache.set(path, manifest);
    }
    return manifest;
}
function evalManifest(path, shouldCache = true, cache = sharedCache) {
    const cached = shouldCache && cache.get(path);
    if (cached) {
        return cached;
    }
    const content = (0, _fs.readFileSync)(path, "utf8");
    if (content.length === 0) {
        throw new Error("Manifest file is empty");
    }
    let contextObject = {};
    (0, _vm.runInNewContext)(content, contextObject);
    // Freeze the context object so it cannot be modified if we're caching it.
    if (shouldCache) {
        contextObject = (0, _deepfreeze.deepFreeze)(contextObject);
    }
    if (shouldCache) {
        cache.set(path, contextObject);
    }
    return contextObject;
}
function clearManifestCache(path, cache = sharedCache) {
    return cache.delete(path);
}

//# sourceMappingURL=load-manifest.js.map