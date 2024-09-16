"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    createDefaultMetadata: null,
    createDefaultViewport: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    createDefaultMetadata: function() {
        return createDefaultMetadata;
    },
    createDefaultViewport: function() {
        return createDefaultViewport;
    }
});
function createDefaultViewport() {
    return {
        // name=viewport
        width: "device-width",
        initialScale: 1,
        // visual metadata
        themeColor: null,
        colorScheme: null
    };
}
function createDefaultMetadata() {
    return {
        // Deprecated ones
        viewport: null,
        themeColor: null,
        colorScheme: null,
        metadataBase: null,
        // Other values are all null
        title: null,
        description: null,
        applicationName: null,
        authors: null,
        generator: null,
        keywords: null,
        referrer: null,
        creator: null,
        publisher: null,
        robots: null,
        manifest: null,
        alternates: {
            canonical: null,
            languages: null,
            media: null,
            types: null
        },
        icons: null,
        openGraph: null,
        twitter: null,
        verification: {},
        appleWebApp: null,
        formatDetection: null,
        itunes: null,
        facebook: null,
        abstract: null,
        appLinks: null,
        archives: null,
        assets: null,
        bookmarks: null,
        category: null,
        classification: null,
        other: {}
    };
}

//# sourceMappingURL=default-metadata.js.map