import type { BuildManifest } from '../../server/get-page-files';
import type { ServerRuntime } from '../../types';
import type { NEXT_DATA } from './utils';
import type { NextFontManifest } from '../../build/webpack/plugins/next-font-manifest-plugin';
import type { DeepReadonly } from './deep-readonly';
import { type JSX } from 'react';
export type HtmlProps = {
    __NEXT_DATA__: NEXT_DATA;
    strictNextHead: boolean;
    dangerousAsPath: string;
    docComponentsRendered: {
        Html?: boolean;
        Main?: boolean;
        Head?: boolean;
        NextScript?: boolean;
    };
    buildManifest: BuildManifest;
    ampPath: string;
    inAmpMode: boolean;
    hybridAmp: boolean;
    isDevelopment: boolean;
    dynamicImports: string[];
    /**
     * This manifest is only needed for Pages dir, Production, Webpack
     * @see https://github.com/vercel/next.js/pull/72959
     */
    dynamicCssManifest: Set<string>;
    assetPrefix?: string;
    canonicalBase: string;
    headTags: any[];
    unstable_runtimeJS?: false;
    unstable_JsPreload?: false;
    assetQueryString: string;
    scriptLoader: {
        afterInteractive?: string[];
        beforeInteractive?: any[];
        worker?: any[];
    };
    locale?: string;
    disableOptimizedLoading?: boolean;
    styles?: React.ReactElement[] | Iterable<React.ReactNode>;
    head?: Array<JSX.Element | null>;
    crossOrigin?: 'anonymous' | 'use-credentials' | '' | undefined;
    optimizeCss?: any;
    nextConfigOutput?: 'standalone' | 'export';
    nextScriptWorkers?: boolean;
    runtime?: ServerRuntime;
    hasConcurrentFeatures?: boolean;
    largePageDataBytes?: number;
    nextFontManifest?: DeepReadonly<NextFontManifest>;
    experimentalClientTraceMetadata?: string[];
};
export declare const HtmlContext: import("react").Context<HtmlProps | undefined>;
export declare function useHtmlContext(): HtmlProps;
