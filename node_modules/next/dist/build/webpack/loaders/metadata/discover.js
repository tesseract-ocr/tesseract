"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    createMetadataExportsCode: null,
    createStaticMetadataFromRoute: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    createMetadataExportsCode: function() {
        return createMetadataExportsCode;
    },
    createStaticMetadataFromRoute: function() {
        return createStaticMetadataFromRoute;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _querystring = require("querystring");
const _ismetadataroute = require("../../../../lib/metadata/is-metadata-route");
const _constants = require("../../../../lib/constants");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const METADATA_TYPE = "metadata";
// Produce all compositions with filename (icon, apple-icon, etc.) with extensions (png, jpg, etc.)
async function enumMetadataFiles(dir, filename, extensions, { metadataResolver, // When set to true, possible filename without extension could: icon, icon0, ..., icon9
numericSuffix }) {
    const collectedFiles = [];
    const possibleFileNames = [
        filename
    ].concat(numericSuffix ? Array(10).fill(0).map((_, index)=>filename + index) : []);
    for (const name of possibleFileNames){
        const resolved = await metadataResolver(dir, name, extensions);
        if (resolved) {
            collectedFiles.push(resolved);
        }
    }
    return collectedFiles;
}
async function createStaticMetadataFromRoute(resolvedDir, { segment, metadataResolver, isRootLayoutOrRootPage, pageExtensions, basePath }) {
    let hasStaticMetadataFiles = false;
    const staticImagesMetadata = {
        icon: [],
        apple: [],
        twitter: [],
        openGraph: [],
        manifest: undefined
    };
    async function collectIconModuleIfExists(type) {
        if (type === "manifest") {
            const staticManifestExtension = [
                "webmanifest",
                "json"
            ];
            const manifestFile = await enumMetadataFiles(resolvedDir, "manifest", staticManifestExtension.concat(pageExtensions), {
                metadataResolver,
                numericSuffix: false
            });
            if (manifestFile.length > 0) {
                hasStaticMetadataFiles = true;
                const { name, ext } = _path.default.parse(manifestFile[0]);
                const extension = staticManifestExtension.includes(ext.slice(1)) ? ext.slice(1) : "webmanifest";
                staticImagesMetadata.manifest = JSON.stringify(`/${name}.${extension}`);
            }
            return;
        }
        const resolvedMetadataFiles = await enumMetadataFiles(resolvedDir, _ismetadataroute.STATIC_METADATA_IMAGES[type].filename, [
            ..._ismetadataroute.STATIC_METADATA_IMAGES[type].extensions,
            ...type === "favicon" ? [] : pageExtensions
        ], {
            metadataResolver,
            numericSuffix: true
        });
        resolvedMetadataFiles.sort((a, b)=>a.localeCompare(b)).forEach((filepath)=>{
            const imageModuleImportSource = `next-metadata-image-loader?${(0, _querystring.stringify)({
                type,
                segment,
                basePath,
                pageExtensions
            })}!${filepath}?${_constants.WEBPACK_RESOURCE_QUERIES.metadata}`;
            const imageModule = `(async (props) => (await import(/* webpackMode: "eager" */ ${JSON.stringify(imageModuleImportSource)})).default(props))`;
            hasStaticMetadataFiles = true;
            if (type === "favicon") {
                staticImagesMetadata.icon.unshift(imageModule);
            } else {
                staticImagesMetadata[type].push(imageModule);
            }
        });
    }
    // Intentionally make these serial to reuse directory access cache.
    await collectIconModuleIfExists("icon");
    await collectIconModuleIfExists("apple");
    await collectIconModuleIfExists("openGraph");
    await collectIconModuleIfExists("twitter");
    if (isRootLayoutOrRootPage) {
        await collectIconModuleIfExists("favicon");
        await collectIconModuleIfExists("manifest");
    }
    return hasStaticMetadataFiles ? staticImagesMetadata : null;
}
function createMetadataExportsCode(metadata) {
    return metadata ? `${METADATA_TYPE}: {
    icon: [${metadata.icon.join(",")}],
    apple: [${metadata.apple.join(",")}],
    openGraph: [${metadata.openGraph.join(",")}],
    twitter: [${metadata.twitter.join(",")}],
    manifest: ${metadata.manifest ? metadata.manifest : "undefined"}
  }` : "";
}

//# sourceMappingURL=discover.js.map