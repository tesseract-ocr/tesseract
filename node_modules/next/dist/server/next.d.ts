import type { Options as DevServerOptions } from './dev/next-dev-server';
import type { Options as ServerOptions } from './next-server';
import type { IncomingMessage, ServerResponse } from 'http';
import type { Duplex } from 'stream';
import type { NextUrlWithParsedQuery } from './request-meta';
import './require-hook';
import './node-polyfill-crypto';
import type { default as NextNodeServer } from './next-server';
import type { ServerFields } from './lib/router-utils/setup-dev-bundler';
export type NextServerOptions = Omit<ServerOptions | DevServerOptions, 'conf'> & Partial<Pick<ServerOptions | DevServerOptions, 'conf'>>;
export type RequestHandler = (req: IncomingMessage, res: ServerResponse, parsedUrl?: NextUrlWithParsedQuery | undefined) => Promise<void>;
export type UpgradeHandler = (req: IncomingMessage, socket: Duplex, head: Buffer) => Promise<void>;
declare const SYMBOL_LOAD_CONFIG: unique symbol;
interface NextWrapperServer {
    options: NextServerOptions;
    hostname: string | undefined;
    port: number | undefined;
    getRequestHandler(): RequestHandler;
    prepare(serverFields?: ServerFields): Promise<void>;
    setAssetPrefix(assetPrefix: string): void;
    close(): Promise<void>;
    getUpgradeHandler(): UpgradeHandler;
    logError(...args: Parameters<NextNodeServer['logError']>): void;
    render(...args: Parameters<NextNodeServer['render']>): ReturnType<NextNodeServer['render']>;
    renderToHTML(...args: Parameters<NextNodeServer['renderToHTML']>): ReturnType<NextNodeServer['renderToHTML']>;
    renderError(...args: Parameters<NextNodeServer['renderError']>): ReturnType<NextNodeServer['renderError']>;
    renderErrorToHTML(...args: Parameters<NextNodeServer['renderErrorToHTML']>): ReturnType<NextNodeServer['renderErrorToHTML']>;
    render404(...args: Parameters<NextNodeServer['render404']>): ReturnType<NextNodeServer['render404']>;
}
/** The wrapper server used by `next start` */
export declare class NextServer implements NextWrapperServer {
    private serverPromise?;
    private server?;
    private reqHandler?;
    private reqHandlerPromise?;
    private preparedAssetPrefix?;
    options: NextServerOptions;
    constructor(options: NextServerOptions);
    get hostname(): string | undefined;
    get port(): number | undefined;
    getRequestHandler(): RequestHandler;
    getUpgradeHandler(): UpgradeHandler;
    setAssetPrefix(assetPrefix: string): void;
    logError(...args: Parameters<NextWrapperServer['logError']>): void;
    render(...args: Parameters<NextWrapperServer['render']>): Promise<void>;
    renderToHTML(...args: Parameters<NextWrapperServer['renderToHTML']>): Promise<string | null>;
    renderError(...args: Parameters<NextWrapperServer['renderError']>): Promise<void>;
    renderErrorToHTML(...args: Parameters<NextWrapperServer['renderErrorToHTML']>): Promise<string | null>;
    render404(...args: Parameters<NextWrapperServer['render404']>): Promise<void>;
    prepare(serverFields?: ServerFields): Promise<void>;
    close(): Promise<void>;
    private createServer;
    private [SYMBOL_LOAD_CONFIG];
    private getServer;
    private getServerRequestHandler;
}
declare function createServer(options: NextServerOptions & {
    turbo?: boolean;
    turbopack?: boolean;
}): NextWrapperServer;
export default createServer;
