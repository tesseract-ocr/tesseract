import type { IncomingMessage } from 'http';
import type { DevBundler } from './router-utils/setup-dev-bundler';
import type { WorkerRequestHandler } from './types';
import { LRUCache } from './lru-cache';
/**
 * The DevBundlerService provides an interface to perform tasks with the
 * bundler while in development.
 */
export declare class DevBundlerService {
    private readonly bundler;
    private readonly handler;
    appIsrManifestInner: InstanceType<typeof LRUCache>;
    constructor(bundler: DevBundler, handler: WorkerRequestHandler);
    ensurePage: typeof this.bundler.hotReloader.ensurePage;
    logErrorWithOriginalStack: (err: unknown, type?: "unhandledRejection" | "uncaughtException" | "warning" | "app-dir") => void;
    getFallbackErrorComponents(url?: string): Promise<void>;
    getCompilationError(page: string): Promise<any>;
    revalidate({ urlPath, revalidateHeaders, opts: revalidateOpts, }: {
        urlPath: string;
        revalidateHeaders: IncomingMessage['headers'];
        opts: any;
    }): Promise<{}>;
    get appIsrManifest(): Record<string, boolean>;
    setAppIsrStatus(key: string, value: boolean | null): void;
}
