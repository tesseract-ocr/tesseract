"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    STATIC_METADATA_IMAGES: null,
    getExtensionRegexString: null,
    isMetadataRoute: null,
    isMetadataRouteFile: null,
    isStaticMetadataRoute: null,
    isStaticMetadataRouteFile: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    STATIC_METADATA_IMAGES: function() {
        return STATIC_METADATA_IMAGES;
    },
    getExtensionRegexString: function() {
        return getExtensionRegexString;
    },
    isMetadataRoute: function() {
        return isMetadataRoute;
    },
    isMetadataRouteFile: function() {
        return isMetadataRouteFile;
    },
    isStaticMetadataRoute: function() {
        return isStaticMetadataRoute;
    },
    isStaticMetadataRouteFile: function() {
        return isStaticMetadataRouteFile;
    }
});
const _normalizepathsep = require("../../shared/lib/page-path/normalize-path-sep");
const STATIC_METADATA_IMAGES = {
    icon: {
        filename: 'icon',
        extensions: [
            'ico',
            'jpg',
            'jpeg',
            'png',
            'svg'
        ]
    },
    apple: {
        filename: 'apple-icon',
        extensions: [
            'jpg',
            'jpeg',
            'png'
        ]
    },
    favicon: {
        filename: 'favicon',
        extensions: [
            'ico'
        ]
    },
    openGraph: {
        filename: 'opengraph-image',
        extensions: [
            'jpg',
            'jpeg',
            'png',
            'gif'
        ]
    },
    twitter: {
        filename: 'twitter-image',
        extensions: [
            'jpg',
            'jpeg',
            'png',
            'gif'
        ]
    }
};
// Match routes that are metadata routes, e.g. /sitemap.xml, /favicon.<ext>, /<icon>.<ext>, etc.
// TODO-METADATA: support more metadata routes with more extensions
const defaultExtensions = [
    'js',
    'jsx',
    'ts',
    'tsx'
];
const getExtensionRegexString = (staticExtensions, dynamicExtensions)=>{
    // If there's no possible multi dynamic routes, will not match any <name>[].<ext> files
    if (!dynamicExtensions) {
        return `\\.(?:${staticExtensions.join('|')})`;
    }
    return `(?:\\.(${staticExtensions.join('|')})|((\\[\\])?\\.(${dynamicExtensions.join('|')})))`;
};
function isMetadataRouteFile(appDirRelativePath, pageExtensions, withExtension) {
    const metadataRouteFilesRegex = [
        new RegExp(`^[\\\\/]robots${withExtension ? `${getExtensionRegexString(pageExtensions.concat('txt'), null)}$` : ''}`),
        new RegExp(`^[\\\\/]manifest${withExtension ? `${getExtensionRegexString(pageExtensions.concat('webmanifest', 'json'), null)}$` : ''}`),
        new RegExp(`^[\\\\/]favicon\\.ico$`),
        new RegExp(`[\\\\/]sitemap${withExtension ? `${getExtensionRegexString([
            'xml'
        ], pageExtensions)}$` : ''}`),
        new RegExp(`[\\\\/]${STATIC_METADATA_IMAGES.icon.filename}\\d?${withExtension ? `${getExtensionRegexString(STATIC_METADATA_IMAGES.icon.extensions, pageExtensions)}$` : ''}`),
        new RegExp(`[\\\\/]${STATIC_METADATA_IMAGES.apple.filename}\\d?${withExtension ? `${getExtensionRegexString(STATIC_METADATA_IMAGES.apple.extensions, pageExtensions)}$` : ''}`),
        new RegExp(`[\\\\/]${STATIC_METADATA_IMAGES.openGraph.filename}\\d?${withExtension ? `${getExtensionRegexString(STATIC_METADATA_IMAGES.openGraph.extensions, pageExtensions)}$` : ''}`),
        new RegExp(`[\\\\/]${STATIC_METADATA_IMAGES.twitter.filename}\\d?${withExtension ? `${getExtensionRegexString(STATIC_METADATA_IMAGES.twitter.extensions, pageExtensions)}$` : ''}`)
    ];
    const normalizedAppDirRelativePath = (0, _normalizepathsep.normalizePathSep)(appDirRelativePath);
    return metadataRouteFilesRegex.some((r)=>r.test(normalizedAppDirRelativePath));
}
function isStaticMetadataRouteFile(appDirRelativePath) {
    return isMetadataRouteFile(appDirRelativePath, [], true);
}
function isStaticMetadataRoute(page) {
    return page === '/robots' || page === '/manifest' || isStaticMetadataRouteFile(page);
}
function isMetadataRoute(route) {
    let page = route.replace(/^\/?app\//, '').replace(/\/route$/, '');
    if (page[0] !== '/') page = '/' + page;
    return !page.endsWith('/page') && isMetadataRouteFile(page, defaultExtensions, false);
}

//# sourceMappingURL=is-metadata-route.js.map