import type { ProxyServer } from './types';
import type { FetchHandler } from './fetch-api';
export declare function createProxyServer({ onFetch, }: {
    onFetch?: FetchHandler;
}): Promise<ProxyServer>;
