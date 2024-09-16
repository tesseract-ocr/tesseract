import { jsx as _jsx, Fragment as _Fragment } from "react/jsx-runtime";
import React from "react";
import { AppleWebAppMeta, FormatDetectionMeta, ItunesMeta, BasicMeta, ViewportMeta, VerificationMeta, FacebookMeta } from "./generate/basic";
import { AlternatesMetadata } from "./generate/alternate";
import { OpenGraphMetadata, TwitterMetadata, AppLinksMeta } from "./generate/opengraph";
import { IconsMetadata } from "./generate/icons";
import { resolveMetadata } from "./resolve-metadata";
import { MetaFilter } from "./generate/meta";
import { createDefaultMetadata, createDefaultViewport } from "./default-metadata";
import { isNotFoundError } from "../../client/components/not-found";
export function createMetadataContext(urlPathname, renderOpts) {
    return {
        pathname: urlPathname.split("?")[0],
        trailingSlash: renderOpts.trailingSlash,
        isStandaloneMode: renderOpts.nextConfigOutput === "standalone"
    };
}
// Use a promise to share the status of the metadata resolving,
// returning two components `MetadataTree` and `MetadataOutlet`
// `MetadataTree` is the one that will be rendered at first in the content sequence for metadata tags.
// `MetadataOutlet` is the one that will be rendered under error boundaries for metadata resolving errors.
// In this way we can let the metadata tags always render successfully,
// and the error will be caught by the error boundary and trigger fallbacks.
export function createMetadataComponents({ tree, query, metadataContext, getDynamicParamFromSegment, appUsingSizeAdjustment, errorType, createDynamicallyTrackedSearchParams }) {
    let resolve;
    // Only use promise.resolve here to avoid unhandled rejections
    const metadataErrorResolving = new Promise((res)=>{
        resolve = res;
    });
    async function MetadataTree() {
        const defaultMetadata = createDefaultMetadata();
        const defaultViewport = createDefaultViewport();
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
        const [resolvedError, resolvedMetadata, resolvedViewport] = await resolveMetadata({
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
            if (!errorType && isNotFoundError(resolvedError)) {
                const [notFoundMetadataError, notFoundMetadata, notFoundViewport] = await resolveMetadata({
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
        const elements = MetaFilter([
            ViewportMeta({
                viewport: viewport
            }),
            BasicMeta({
                metadata
            }),
            AlternatesMetadata({
                alternates: metadata.alternates
            }),
            ItunesMeta({
                itunes: metadata.itunes
            }),
            FacebookMeta({
                facebook: metadata.facebook
            }),
            FormatDetectionMeta({
                formatDetection: metadata.formatDetection
            }),
            VerificationMeta({
                verification: metadata.verification
            }),
            AppleWebAppMeta({
                appleWebApp: metadata.appleWebApp
            }),
            OpenGraphMetadata({
                openGraph: metadata.openGraph
            }),
            TwitterMetadata({
                twitter: metadata.twitter
            }),
            AppLinksMeta({
                appLinks: metadata.appLinks
            }),
            IconsMetadata({
                icons: metadata.icons
            })
        ]);
        if (appUsingSizeAdjustment) elements.push(/*#__PURE__*/ _jsx("meta", {
            name: "next-size-adjust"
        }));
        return /*#__PURE__*/ _jsx(_Fragment, {
            children: elements.map((el, index)=>{
                return /*#__PURE__*/ React.cloneElement(el, {
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