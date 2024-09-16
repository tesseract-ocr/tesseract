/// <reference types="node" />
import type { IncomingMessage } from 'http';
import type { DevBundler } from './router-utils/setup-dev-bundler';
import type { WorkerRequestHandler } from './types';
/**
 * The DevBundlerService provides an interface to perform tasks with the
 * bundler while in development.
 */
export declare class DevBundlerService {
    private readonly bundler;
    private readonly handler;
    constructor(bundler: DevBundler, handler: WorkerRequestHandler);
    ensurePage: typeof this.bundler.hotReloader.ensurePage;
    logErrorWithOriginalStack: typeof this.bundler.logErrorWithOriginalStack;
    getFallbackErrorComponents(url?: string): Promise<void>;
    getCompilationError(page: string): Promise<any>;
    revalidate({ urlPath, revalidateHeaders, opts: revalidateOpts, }: {
        urlPath: string;
        revalidateHeaders: IncomingMessage['headers'];
        opts: any;
    }): Promise<{}>;
}
