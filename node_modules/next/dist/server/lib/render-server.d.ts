import type { NextServer, RequestHandler } from '../next';
import type { DevBundlerService } from './dev-bundler-service';
import type { PropagateToWorkersField } from './router-utils/types';
import type { Span } from '../../trace';
export declare function clearAllModuleContexts(): Promise<void> | undefined;
export declare function clearModuleContext(target: string): Promise<void> | undefined;
export declare function deleteAppClientCache(): void | undefined;
export declare function deleteCache(filePaths: string[]): void;
export declare function propagateServerField(dir: string, field: PropagateToWorkersField, value: any): Promise<void>;
declare function initializeImpl(opts: {
    dir: string;
    port: number;
    dev: boolean;
    minimalMode?: boolean;
    hostname?: string;
    isNodeDebugging: boolean;
    keepAliveTimeout?: number;
    serverFields?: any;
    server?: any;
    experimentalTestProxy: boolean;
    experimentalHttpsServer: boolean;
    _ipcPort?: string;
    _ipcKey?: string;
    bundlerService: DevBundlerService | undefined;
    startServerSpan: Span | undefined;
}): Promise<{
    requestHandler: RequestHandler;
    upgradeHandler: any;
    app: NextServer;
}>;
export declare function initialize(opts: Parameters<typeof initializeImpl>[0]): Promise<{
    requestHandler: ReturnType<InstanceType<typeof NextServer>['getRequestHandler']>;
    upgradeHandler: ReturnType<InstanceType<typeof NextServer>['getUpgradeHandler']>;
    app: NextServer;
}>;
export {};
