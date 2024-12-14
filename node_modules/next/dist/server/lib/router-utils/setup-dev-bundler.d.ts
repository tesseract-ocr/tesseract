import type { NextConfigComplete } from '../../config-shared';
import type { UnwrapPromise } from '../../../lib/coalesced-function';
import { type MiddlewareMatcher } from '../../../build/analysis/get-page-static-info';
import type { MiddlewareRouteMatch } from '../../../shared/lib/router/utils/middleware-route-matcher';
import type { PropagateToWorkersField } from './types';
import type { NextJsHotReloaderInterface } from '../../dev/hot-reloader-types';
import type { Telemetry } from '../../../telemetry/storage';
import type { IncomingMessage, ServerResponse } from 'http';
import type { LazyRenderServerInstance } from '../router-server';
export type SetupOpts = {
    renderServer: LazyRenderServerInstance;
    dir: string;
    turbo?: boolean;
    appDir?: string;
    pagesDir?: string;
    telemetry: Telemetry;
    isCustomServer?: boolean;
    fsChecker: UnwrapPromise<ReturnType<typeof import('./filesystem').setupFsCheck>>;
    nextConfig: NextConfigComplete;
    port: number;
    onDevServerCleanup: ((listener: () => Promise<void>) => void) | undefined;
    resetFetch: () => void;
};
export type ServerFields = {
    actualMiddlewareFile?: string | undefined;
    actualInstrumentationHookFile?: string | undefined;
    appPathRoutes?: Record<string, string | string[]>;
    middleware?: {
        page: string;
        match: MiddlewareRouteMatch;
        matchers?: MiddlewareMatcher[];
    } | undefined;
    hasAppNotFound?: boolean;
    interceptionRoutes?: ReturnType<typeof import('./filesystem').buildCustomRoute>[];
    setAppIsrStatus?: (key: string, value: boolean) => void;
    resetFetch?: () => void;
};
export declare function propagateServerField(opts: SetupOpts, field: PropagateToWorkersField, args: any): Promise<void>;
export declare function setupDevBundler(opts: SetupOpts): Promise<{
    serverFields: ServerFields;
    hotReloader: NextJsHotReloaderInterface;
    requestHandler: (req: IncomingMessage, res: ServerResponse) => Promise<{
        finished: boolean;
    }>;
    logErrorWithOriginalStack: (err: unknown, type?: "unhandledRejection" | "uncaughtException" | "warning" | "app-dir") => void;
    ensureMiddleware(requestUrl?: string): Promise<void>;
}>;
export type DevBundler = Awaited<ReturnType<typeof setupDevBundler>>;
