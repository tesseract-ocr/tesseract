import { readFileSync } from "fs";
import { runInNewContext } from "vm";
import { deepFreeze } from "../shared/lib/deep-freeze";
const sharedCache = new Map();
export function loadManifest(path, shouldCache = true, cache = sharedCache) {
    const cached = shouldCache && cache.get(path);
    if (cached) {
        return cached;
    }
    let manifest = JSON.parse(readFileSync(path, "utf8"));
    // Freeze the manifest so it cannot be modified if we're caching it.
    if (shouldCache) {
        manifest = deepFreeze(manifest);
    }
    if (shouldCache) {
        cache.set(path, manifest);
    }
    return manifest;
}
export function evalManifest(path, shouldCache = true, cache = sharedCache) {
    const cached = shouldCache && cache.get(path);
    if (cached) {
        return cached;
    }
    const content = readFileSync(path, "utf8");
    if (content.length === 0) {
        throw new Error("Manifest file is empty");
    }
    let contextObject = {};
    runInNewContext(content, contextObject);
    // Freeze the context object so it cannot be modified if we're caching it.
    if (shouldCache) {
        contextObject = deepFreeze(contextObject);
    }
    if (shouldCache) {
        cache.set(path, contextObject);
    }
    return contextObject;
}
export function clearManifestCache(path, cache = sharedCache) {
    return cache.delete(path);
}

//# sourceMappingURL=load-manifest.js.map