import type { Addresses, RestoreOriginalFunction } from './types';
/**
 * Proxy the TCP connect method to determine if any network access is made during the build
 *
 * @param addresses An array to track the addresses that are accessed.
 * @returns A function that restores the original connect method.
 */
export declare function tcpProxy(addresses: Addresses): RestoreOriginalFunction;
