"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "createMetadataComponents", {
    enumerable: true,
    get: function() {
        return createMetadataComponents;
    }
});
const _jsxruntime = require("react/jsx-runtime");
const _react = require("react");
const _basic = require("./generate/basic");
const _alternate = require("./generate/alternate");
const _opengraph = require("./generate/opengraph");
const _icons = require("./generate/icons");
const _resolvemetadata = require("./resolve-metadata");
const _meta = require("./generate/meta");
const _httpaccessfallback = require("../../client/components/http-access-fallback/http-access-fallback");
const _metadataconstants = require("./metadata-constants");
function createMetadataComponents({ tree, searchParams, metadataContext, getDynamicParamFromSegment, appUsingSizeAdjustment, errorType, createServerParamsForMetadata, workStore, MetadataBoundary, ViewportBoundary }) {
    function MetadataRoot() {
        return /*#__PURE__*/ (0, _jsxruntime.jsxs)(_jsxruntime.Fragment, {
            children: [
                /*#__PURE__*/ (0, _jsxruntime.jsx)(MetadataBoundary, {
                    children: /*#__PURE__*/ (0, _jsxruntime.jsx)(Metadata, {})
                }),
                /*#__PURE__*/ (0, _jsxruntime.jsx)(ViewportBoundary, {
                    children: /*#__PURE__*/ (0, _jsxruntime.jsx)(Viewport, {})
                }),
                appUsingSizeAdjustment ? /*#__PURE__*/ (0, _jsxruntime.jsx)("meta", {
                    name: "next-size-adjust",
                    content: ""
                }) : null
            ]
        });
    }
    async function viewport() {
        return getResolvedViewport(tree, searchParams, getDynamicParamFromSegment, createServerParamsForMetadata, workStore, errorType);
    }
    async function Viewport() {
        try {
            return await viewport();
        } catch (error) {
            if (!errorType && (0, _httpaccessfallback.isHTTPAccessFallbackError)(error)) {
                try {
                    return await getNotFoundViewport(tree, searchParams, getDynamicParamFromSegment, createServerParamsForMetadata, workStore);
                } catch  {}
            }
            // We don't actually want to error in this component. We will
            // also error in the MetadataOutlet which causes the error to
            // bubble from the right position in the page to be caught by the
            // appropriate boundaries
            return null;
        }
    }
    Viewport.displayName = _metadataconstants.VIEWPORT_BOUNDARY_NAME;
    async function metadata() {
        return getResolvedMetadata(tree, searchParams, getDynamicParamFromSegment, metadataContext, createServerParamsForMetadata, workStore, errorType);
    }
    async function Metadata() {
        try {
            return await metadata();
        } catch (error) {
            if (!errorType && (0, _httpaccessfallback.isHTTPAccessFallbackError)(error)) {
                try {
                    return await getNotFoundMetadata(tree, searchParams, getDynamicParamFromSegment, metadataContext, createServerParamsForMetadata, workStore);
                } catch  {}
            }
            // We don't actually want to error in this component. We will
            // also error in the MetadataOutlet which causes the error to
            // bubble from the right position in the page to be caught by the
            // appropriate boundaries
            return null;
        }
    }
    Metadata.displayName = _metadataconstants.METADATA_BOUNDARY_NAME;
    async function getMetadataAndViewportReady() {
        await viewport();
        await metadata();
        return undefined;
    }
    return [
        MetadataRoot,
        getMetadataAndViewportReady
    ];
}
const getResolvedMetadata = (0, _react.cache)(getResolvedMetadataImpl);
async function getResolvedMetadataImpl(tree, searchParams, getDynamicParamFromSegment, metadataContext, createServerParamsForMetadata, workStore, errorType) {
    const errorConvention = errorType === 'redirect' ? undefined : errorType;
    const metadataItems = await (0, _resolvemetadata.resolveMetadataItems)(tree, searchParams, errorConvention, getDynamicParamFromSegment, createServerParamsForMetadata, workStore);
    const elements = createMetadataElements(await (0, _resolvemetadata.accumulateMetadata)(metadataItems, metadataContext));
    return /*#__PURE__*/ (0, _jsxruntime.jsx)(_jsxruntime.Fragment, {
        children: elements.map((el, index)=>{
            return /*#__PURE__*/ (0, _react.cloneElement)(el, {
                key: index
            });
        })
    });
}
const getNotFoundMetadata = (0, _react.cache)(getNotFoundMetadataImpl);
async function getNotFoundMetadataImpl(tree, searchParams, getDynamicParamFromSegment, metadataContext, createServerParamsForMetadata, workStore) {
    const notFoundErrorConvention = 'not-found';
    const notFoundMetadataItems = await (0, _resolvemetadata.resolveMetadataItems)(tree, searchParams, notFoundErrorConvention, getDynamicParamFromSegment, createServerParamsForMetadata, workStore);
    const elements = createMetadataElements(await (0, _resolvemetadata.accumulateMetadata)(notFoundMetadataItems, metadataContext));
    return /*#__PURE__*/ (0, _jsxruntime.jsx)(_jsxruntime.Fragment, {
        children: elements.map((el, index)=>{
            return /*#__PURE__*/ (0, _react.cloneElement)(el, {
                key: index
            });
        })
    });
}
const getResolvedViewport = (0, _react.cache)(getResolvedViewportImpl);
async function getResolvedViewportImpl(tree, searchParams, getDynamicParamFromSegment, createServerParamsForMetadata, workStore, errorType) {
    const errorConvention = errorType === 'redirect' ? undefined : errorType;
    const metadataItems = await (0, _resolvemetadata.resolveMetadataItems)(tree, searchParams, errorConvention, getDynamicParamFromSegment, createServerParamsForMetadata, workStore);
    const elements = createViewportElements(await (0, _resolvemetadata.accumulateViewport)(metadataItems));
    return /*#__PURE__*/ (0, _jsxruntime.jsx)(_jsxruntime.Fragment, {
        children: elements.map((el, index)=>{
            return /*#__PURE__*/ (0, _react.cloneElement)(el, {
                key: index
            });
        })
    });
}
const getNotFoundViewport = (0, _react.cache)(getNotFoundViewportImpl);
async function getNotFoundViewportImpl(tree, searchParams, getDynamicParamFromSegment, createServerParamsForMetadata, workStore) {
    const notFoundErrorConvention = 'not-found';
    const notFoundMetadataItems = await (0, _resolvemetadata.resolveMetadataItems)(tree, searchParams, notFoundErrorConvention, getDynamicParamFromSegment, createServerParamsForMetadata, workStore);
    const elements = createViewportElements(await (0, _resolvemetadata.accumulateViewport)(notFoundMetadataItems));
    return /*#__PURE__*/ (0, _jsxruntime.jsx)(_jsxruntime.Fragment, {
        children: elements.map((el, index)=>{
            return /*#__PURE__*/ (0, _react.cloneElement)(el, {
                key: index
            });
        })
    });
}
function createMetadataElements(metadata) {
    return (0, _meta.MetaFilter)([
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
}
function createViewportElements(viewport) {
    return (0, _meta.MetaFilter)([
        (0, _basic.ViewportMeta)({
            viewport: viewport
        })
    ]);
}

//# sourceMappingURL=metadata.js.map