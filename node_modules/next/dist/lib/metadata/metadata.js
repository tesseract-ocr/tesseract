"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    createMetadataComponents: null,
    createMetadataContext: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    createMetadataComponents: function() {
        return createMetadataComponents;
    },
    createMetadataContext: function() {
        return createMetadataContext;
    }
});
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_default(require("react"));
const _basic = require("./generate/basic");
const _alternate = require("./generate/alternate");
const _opengraph = require("./generate/opengraph");
const _icons = require("./generate/icons");
const _resolvemetadata = require("./resolve-metadata");
const _meta = require("./generate/meta");
const _defaultmetadata = require("./default-metadata");
const _notfound = require("../../client/components/not-found");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function createMetadataContext(urlPathname, renderOpts) {
    return {
        pathname: urlPathname.split("?")[0],
        trailingSlash: renderOpts.trailingSlash,
        isStandaloneMode: renderOpts.nextConfigOutput === "standalone"
    };
}
function createMetadataComponents({ tree, query, metadataContext, getDynamicParamFromSegment, appUsingSizeAdjustment, errorType, createDynamicallyTrackedSearchParams }) {
    let resolve;
    // Only use promise.resolve here to avoid unhandled rejections
    const metadataErrorResolving = new Promise((res)=>{
        resolve = res;
    });
    async function MetadataTree() {
        const defaultMetadata = (0, _defaultmetadata.createDefaultMetadata)();
        const defaultViewport = (0, _defaultmetadata.createDefaultViewport)();
        let metadata = defaultMetadata;
        let viewport = defaultViewport;
        let error;
        const errorMetadataItem = [
            null,
            null,
            null
        ];
        const errorConvention = errorType === "redirect" ? undefined : errorType;
        const searchParams = createDynamicallyTrackedSearchParams(query);
        const [resolvedError, resolvedMetadata, resolvedViewport] = await (0, _resolvemetadata.resolveMetadata)({
            tree,
            parentParams: {},
            metadataItems: [],
            errorMetadataItem,
            searchParams,
            getDynamicParamFromSegment,
            errorConvention,
            metadataContext
        });
        if (!resolvedError) {
            viewport = resolvedViewport;
            metadata = resolvedMetadata;
            resolve(undefined);
        } else {
            error = resolvedError;
            // If the error triggers in initial metadata resolving, re-resolve with proper error type.
            // They'll be saved for flight data, when hydrates, it will replaces the SSR'd metadata with this.
            // for not-found error: resolve not-found metadata
            if (!errorType && (0, _notfound.isNotFoundError)(resolvedError)) {
                const [notFoundMetadataError, notFoundMetadata, notFoundViewport] = await (0, _resolvemetadata.resolveMetadata)({
                    tree,
                    parentParams: {},
                    metadataItems: [],
                    errorMetadataItem,
                    searchParams,
                    getDynamicParamFromSegment,
                    errorConvention: "not-found",
                    metadataContext
                });
                viewport = notFoundViewport;
                metadata = notFoundMetadata;
                error = notFoundMetadataError || error;
            }
            resolve(error);
        }
        const elements = (0, _meta.MetaFilter)([
            (0, _basic.ViewportMeta)({
                viewport: viewport
            }),
            (0, _basic.BasicMeta)({
                metadata
            }),
            (0, _alternate.AlternatesMetadata)({
                alternates: metadata.alternates
            }),
            (0, _basic.ItunesMeta)({
                itunes: metadata.itunes
            }),
            (0, _basic.FacebookMeta)({
                facebook: metadata.facebook
            }),
            (0, _basic.FormatDetectionMeta)({
                formatDetection: metadata.formatDetection
            }),
            (0, _basic.VerificationMeta)({
                verification: metadata.verification
            }),
            (0, _basic.AppleWebAppMeta)({
                appleWebApp: metadata.appleWebApp
            }),
            (0, _opengraph.OpenGraphMetadata)({
                openGraph: metadata.openGraph
            }),
            (0, _opengraph.TwitterMetadata)({
                twitter: metadata.twitter
            }),
            (0, _opengraph.AppLinksMeta)({
                appLinks: metadata.appLinks
            }),
            (0, _icons.IconsMetadata)({
                icons: metadata.icons
            })
        ]);
        if (appUsingSizeAdjustment) elements.push(/*#__PURE__*/ (0, _jsxruntime.jsx)("meta", {
            name: "next-size-adjust"
        }));
        return /*#__PURE__*/ (0, _jsxruntime.jsx)(_jsxruntime.Fragment, {
            children: elements.map((el, index)=>{
                return /*#__PURE__*/ _react.default.cloneElement(el, {
                    key: index
                });
            })
        });
    }
    async function MetadataOutlet() {
        const error = await metadataErrorResolving;
        if (error) {
            throw error;
        }
        return null;
    }
    return [
        MetadataTree,
        MetadataOutlet
    ];
}

//# sourceMappingURL=metadata.js.map