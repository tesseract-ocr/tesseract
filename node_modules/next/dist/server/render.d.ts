import type { IncomingMessage, ServerResponse } from 'http';
import type { ParsedUrlQuery } from 'querystring';
import type { DomainLocale } from './config';
import type { AppType, DocumentType } from '../shared/lib/utils';
import type { ImageConfigComplete } from '../shared/lib/image-config';
import { type __ApiPreviewProps } from './api-utils';
import type { LoadComponentsReturnType } from './load-components';
import type { ServerRuntime, SizeLimit } from '../types';
import type { ClientReferenceManifest } from '../build/webpack/plugins/flight-manifest-plugin';
import type { NextFontManifest } from '../build/webpack/plugins/next-font-manifest-plugin';
import type { PagesModule } from './route-modules/pages/module';
import type { NextParsedUrlQuery } from './request-meta';
import type { ExpireTime } from './lib/revalidate';
import React from 'react';
import RenderResult from './render-result';
import type { DeepReadonly } from '../shared/lib/deep-readonly';
export type RenderOptsPartial = {
    buildId: string;
    canonicalBase: string;
    runtimeConfig?: {
        [key: string]: any;
    };
    assetPrefix?: string;
    err?: Error | null;
    nextExport?: boolean;
    dev?: boolean;
    ampPath?: string;
    ErrorDebug?: React.ComponentType<{
        error: Error;
    }>;
    ampValidator?: (html: string, pathname: string) => Promise<void>;
    ampSkipValidation?: boolean;
    ampOptimizerConfig?: {
        [key: string]: any;
    };
    isNextDataRequest?: boolean;
    params?: ParsedUrlQuery;
    previewProps: __ApiPreviewProps | undefined;
    basePath: string;
    unstable_runtimeJS?: false;
    unstable_JsPreload?: false;
    optimizeCss: any;
    nextConfigOutput?: 'standalone' | 'export';
    nextScriptWorkers: any;
    assetQueryString?: string;
    resolvedUrl?: string;
    resolvedAsPath?: string;
    clientReferenceManifest?: DeepReadonly<ClientReferenceManifest>;
    nextFontManifest?: DeepReadonly<NextFontManifest>;
    distDir?: string;
    locale?: string;
    locales?: string[];
    defaultLocale?: string;
    domainLocales?: DomainLocale[];
    disableOptimizedLoading?: boolean;
    supportsDynamicResponse: boolean;
    isBot?: boolean;
    runtime?: ServerRuntime;
    serverComponents?: boolean;
    serverActions?: {
        bodySizeLimit?: SizeLimit;
        allowedOrigins?: string[];
    };
    customServer?: boolean;
    crossOrigin?: 'anonymous' | 'use-credentials' | '' | undefined;
    images: ImageConfigComplete;
    largePageDataBytes?: number;
    isOnDemandRevalidate?: boolean;
    strictNextHead: boolean;
    isDraftMode?: boolean;
    deploymentId?: string;
    isServerAction?: boolean;
    isExperimentalCompile?: boolean;
    isPrefetch?: boolean;
    expireTime?: ExpireTime;
    experimental: {
        clientTraceMetadata?: string[];
    };
};
export type RenderOpts = LoadComponentsReturnType<PagesModule> & RenderOptsPartial;
/**
 * RenderOptsExtra is being used to split away functionality that's within the
 * renderOpts. Eventually we can have more explicit render options for each
 * route kind.
 */
export type RenderOptsExtra = {
    App: AppType;
    Document: DocumentType;
};
export declare function errorToJSON(err: Error): {
    name: string;
    source: "server" | "edge-server";
    message: string;
    stack: string | undefined;
    digest: any;
};
export declare function renderToHTMLImpl(req: IncomingMessage, res: ServerResponse, pathname: string, query: NextParsedUrlQuery, renderOpts: Omit<RenderOpts, keyof RenderOptsExtra>, extra: RenderOptsExtra): Promise<RenderResult>;
export type PagesRender = (req: IncomingMessage, res: ServerResponse, pathname: string, query: NextParsedUrlQuery, renderOpts: RenderOpts) => Promise<RenderResult>;
export declare const renderToHTML: PagesRender;
