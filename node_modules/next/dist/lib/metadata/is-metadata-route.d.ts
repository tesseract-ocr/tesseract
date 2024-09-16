import type { PageExtensions } from '../../build/page-extensions-type';
export declare const STATIC_METADATA_IMAGES: {
    readonly icon: {
        readonly filename: "icon";
        readonly extensions: readonly ["ico", "jpg", "jpeg", "png", "svg"];
    };
    readonly apple: {
        readonly filename: "apple-icon";
        readonly extensions: readonly ["jpg", "jpeg", "png"];
    };
    readonly favicon: {
        readonly filename: "favicon";
        readonly extensions: readonly ["ico"];
    };
    readonly openGraph: {
        readonly filename: "opengraph-image";
        readonly extensions: readonly ["jpg", "jpeg", "png", "gif"];
    };
    readonly twitter: {
        readonly filename: "twitter-image";
        readonly extensions: readonly ["jpg", "jpeg", "png", "gif"];
    };
};
export declare function isMetadataRouteFile(appDirRelativePath: string, pageExtensions: PageExtensions, withExtension: boolean): boolean;
export declare function isStaticMetadataRouteFile(appDirRelativePath: string): boolean;
export declare function isStaticMetadataRoute(page: string): boolean;
export declare function isMetadataRoute(route: string): boolean;
