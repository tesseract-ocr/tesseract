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
export declare function getRequestHandlers({ dir, port, isDev, onDevServerCleanup, server, hostname, minimalMode, keepAliveTimeout, experimentalHttpsServer, quiet, }: {
    dir: string;
    port: number;
    isDev: boolean;
    onDevServerCleanup: ((listener: () => Promise<void>) => void) | undefined;
    server?: import('http').Server;
    hostname?: string;
    minimalMode?: boolean;
    keepAliveTimeout?: number;
    experimentalHttpsServer?: boolean;
    quiet?: boolean;
}): ReturnType<typeof initialize>;
export declare function startServer(serverOptions: StartServerOptions): Promise<void>;
