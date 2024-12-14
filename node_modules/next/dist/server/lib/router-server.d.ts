import '../node-environment';
import '../require-hook';
import { type Span } from '../../trace';
import type { ServerInitResult } from './render-server';
export type RenderServer = Pick<typeof import('./render-server'), 'initialize' | 'clearModuleContext' | 'propagateServerField' | 'getServerField'>;
export interface LazyRenderServerInstance {
    instance?: RenderServer;
}
export declare function initialize(opts: {
    dir: string;
    port: number;
    dev: boolean;
    onDevServerCleanup: ((listener: () => Promise<void>) => void) | undefined;
    server?: import('http').Server;
    minimalMode?: boolean;
    hostname?: string;
    keepAliveTimeout?: number;
    customServer?: boolean;
    experimentalHttpsServer?: boolean;
    startServerSpan?: Span;
    quiet?: boolean;
}): Promise<ServerInitResult>;
