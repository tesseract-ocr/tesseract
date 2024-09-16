import type { EnvVars, RestoreOriginalFunction } from './types';
/**
 * Proxy the environment to track environment variables keys that
 * are accessed during the build.
 *
 * @param envVars A set to track environment variable keys that are accessed.
 * @returns A function that restores the original environment.
 */
export declare function envProxy(envVars: EnvVars): RestoreOriginalFunction;
