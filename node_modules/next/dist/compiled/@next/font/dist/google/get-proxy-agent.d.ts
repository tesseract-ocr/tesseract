/// <reference types="node" />
import type { Agent } from 'https';
/**
 * If the http(s)_proxy environment variables is set, return a proxy agent.
 */
export declare function getProxyAgent(): Agent | undefined;
