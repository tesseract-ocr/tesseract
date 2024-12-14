import type { NextServer, RequestHandler, UpgradeHandler } from '../next';
import type { DevBundlerService } from './dev-bundler-service';
import type { PropagateToWorkersField } from './router-utils/types';
import type { Span } from '../../trace';
export type ServerInitResult = {
    requestHandler: RequestHandler;
    upgradeHandler: UpgradeHandler;
    server: NextServer;
};
export declare function clearAllModuleContexts(): Promise<void> | undefined;
export declare function clearModuleContext(target: string): Promise<void> | undefined;
export declare function getServerField(dir: string, field: PropagateToWorkersField): Promise<string | number | import("../route-matcher-managers/route-matcher-manager").RouteMatcherManager | ((prefix?: string) => void) | (() => Promise<void>) | ((err: Error) => void) | (() => import("../next-server").NodeRequestHandler) | (() => Promise<void>) | ((req: import("../base-http/node").NodeNextRequest | import("http").IncomingMessage, res: import("../base-http/node").NodeNextResponse | import("http").ServerResponse, pathname: string, query?: import("../request-meta").NextParsedUrlQuery, parsedUrl?: import("../request-meta").NextUrlWithParsedQuery, internal?: boolean) => Promise<void>) | ((req: import("../base-http/node").NodeNextRequest | import("http").IncomingMessage, res: import("../base-http/node").NodeNextResponse | import("http").ServerResponse, pathname: string, query?: import("querystring").ParsedUrlQuery) => Promise<string | null>) | ((err: Error | null, req: import("../base-http/node").NodeNextRequest | import("http").IncomingMessage, res: import("../base-http/node").NodeNextResponse | import("http").ServerResponse, pathname: string, query?: import("../request-meta").NextParsedUrlQuery, setHeaders?: boolean) => Promise<void>) | ((err: Error | null, req: import("../base-http/node").NodeNextRequest | import("http").IncomingMessage, res: import("../base-http/node").NodeNextResponse | import("http").ServerResponse, pathname: string, query?: import("querystring").ParsedUrlQuery) => Promise<string | null>) | ((req: import("../base-http/node").NodeNextRequest | import("http").IncomingMessage, res: import("../base-http/node").NodeNextResponse | import("http").ServerResponse, parsedUrl?: import("../request-meta").NextUrlWithParsedQuery, setHeaders?: boolean) => Promise<void>) | (() => Promise<void>) | ((req: import("../base-http/node").NodeNextRequest, res: import("../base-http/node").NodeNextResponse, parsedUrl?: import("../request-meta").NextUrlWithParsedQuery) => Promise<void>) | (({ urlPath, revalidateHeaders, opts, }: {
    urlPath: string;
    revalidateHeaders: {
        [key: string]: string | string[];
    };
    opts: {
        unstable_onlyGenerated?: boolean;
    };
}) => Promise<void>) | ((meta: import("../request-meta").RequestMeta) => import("../base-server").BaseRequestHandler<import("../base-http/node").NodeNextRequest, import("../base-http/node").NodeNextResponse>) | undefined>;
export declare function propagateServerField(dir: string, field: PropagateToWorkersField, value: any): Promise<void>;
declare function initializeImpl(opts: {
    dir: string;
    port: number;
    dev: boolean;
    minimalMode?: boolean;
    hostname?: string;
    keepAliveTimeout?: number;
    serverFields?: any;
    server?: any;
    experimentalTestProxy: boolean;
    experimentalHttpsServer: boolean;
    _ipcPort?: string;
    _ipcKey?: string;
    bundlerService: DevBundlerService | undefined;
    startServerSpan: Span | undefined;
    quiet?: boolean;
    onDevServerCleanup: ((listener: () => Promise<void>) => void) | undefined;
}): Promise<ServerInitResult>;
export declare function initialize(opts: Parameters<typeof initializeImpl>[0]): Promise<ServerInitResult>;
export {};
