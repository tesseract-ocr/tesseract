import type { ResolvedMetadata, ResolvedViewport } from '../types/metadata-interface';
import React from 'react';
export declare function ViewportMeta({ viewport }: {
    viewport: ResolvedViewport;
}): React.ReactElement<any, string | React.JSXElementConstructor<any>>[];
export declare function BasicMeta({ metadata }: {
    metadata: ResolvedMetadata;
}): NonNullable<import("react/jsx-runtime").JSX.Element | (import("react/jsx-runtime").JSX.Element | null)[]>[];
export declare function ItunesMeta({ itunes }: {
    itunes: ResolvedMetadata['itunes'];
}): import("react/jsx-runtime").JSX.Element | null;
export declare function FacebookMeta({ facebook, }: {
    facebook: ResolvedMetadata['facebook'];
}): import("react/jsx-runtime").JSX.Element[] | null;
export declare function FormatDetectionMeta({ formatDetection, }: {
    formatDetection: ResolvedMetadata['formatDetection'];
}): import("react/jsx-runtime").JSX.Element | null;
export declare function AppleWebAppMeta({ appleWebApp, }: {
    appleWebApp: ResolvedMetadata['appleWebApp'];
}): NonNullable<React.ReactElement<any, string | React.JSXElementConstructor<any>> | import("react/jsx-runtime").JSX.Element[]>[] | null;
export declare function VerificationMeta({ verification, }: {
    verification: ResolvedMetadata['verification'];
}): NonNullable<React.ReactElement<any, string | React.JSXElementConstructor<any>> | React.ReactElement<any, string | React.JSXElementConstructor<any>>[]>[][] | null;
