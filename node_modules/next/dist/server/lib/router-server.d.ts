import type { WorkerRequestHandler, WorkerUpgradeHandler } from './types';
import type { NextServer } from '../next';
import '../node-environment';
import '../require-hook';
import { type Span } from '../../trace';
export type RenderServer = Pick<typeof import('./render-server'), 'initialize' | 'deleteCache' | 'clearModuleContext' | 'deleteAppClientCache' | 'propagateServerField'>;
export interface LazyRenderServerInstance {
    instance?: RenderServer;
}
export declare function initialize(opts: {
    dir: string;
    port: number;
    dev: boolean;
    server?: import('http').Server;
    minimalMode?: boolean;
    hostname?: string;
    isNodeDebugging: boolean;
    keepAliveTimeout?: number;
    customServer?: boolean;
    experimentalHttpsServer?: boolean;
    startServerSpan?: Span;
}): Promise<[WorkerRequestHandler, WorkerUpgradeHandler, NextServer]>;
