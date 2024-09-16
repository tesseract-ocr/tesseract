import '../next';
import '../require-hook';
import type { SelfSignedCertificate } from '../../lib/mkcert';
import { initialize } from './router-server';
export interface StartServerOptions {
    dir: string;
    port: number;
    isDev: boolean;
    hostname?: string;
    allowRetry?: boolean;
    customServer?: boolean;
    minimalMode?: boolean;
    keepAliveTimeout?: number;
    selfSignedCertificate?: SelfSignedCertificate;
}
export declare function getRequestHandlers({ dir, port, isDev, server, hostname, minimalMode, isNodeDebugging, keepAliveTimeout, experimentalHttpsServer, }: {
    dir: string;
    port: number;
    isDev: boolean;
    server?: import('http').Server;
    hostname?: string;
    minimalMode?: boolean;
    isNodeDebugging?: boolean;
    keepAliveTimeout?: number;
    experimentalHttpsServer?: boolean;
}): ReturnType<typeof initialize>;
export declare function startServer(serverOptions: StartServerOptions): Promise<void>;
