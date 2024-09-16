/**
 * Formats the manifest depending on the environment variable
 * `NODE_ENV`. If it's set to `development`, it will return a pretty printed
 * JSON string, otherwise it will return a minified JSON string.
 */ "use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "formatManifest", {
    enumerable: true,
    get: function() {
        return formatManifest;
    }
});
function formatManifest(manifest) {
    if (process.env.NODE_ENV === "development") {
        return JSON.stringify(manifest, null, 2);
    }
    return JSON.stringify(manifest);
}

//# sourceMappingURL=format-manifest.js.map