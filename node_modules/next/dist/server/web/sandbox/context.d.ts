/// <reference types="node" />
import type { EdgeFunctionDefinition } from '../../../build/webpack/plugins/middleware-plugin';
import { AsyncLocalStorage } from 'async_hooks';
import { EdgeRuntime } from 'next/dist/compiled/edge-runtime';
/**
 * Same as clearModuleContext but for all module contexts.
 */
export declare function clearAllModuleContexts(): Promise<void>;
/**
 * For a given path a context, this function checks if there is any module
 * context that contains the path with an older content and, if that's the
 * case, removes the context from the cache.
 *
 * This function also clears all intervals and timeouts created by the
 * module context.
 */
export declare function clearModuleContext(path: string): Promise<void>;
export declare const requestStore: AsyncLocalStorage<{
    headers: Headers;
}>;
interface ModuleContextOptions {
    moduleName: string;
    onError: (err: unknown) => void;
    onWarning: (warn: Error) => void;
    useCache: boolean;
    distDir: string;
    edgeFunctionEntry: Pick<EdgeFunctionDefinition, 'assets' | 'wasm' | 'env'>;
}
/**
 * For a given module name this function will get a cached module
 * context or create it. It will return the module context along
 * with a function that allows to run some code from a given
 * filepath within the context.
 */
export declare function getModuleContext(options: ModuleContextOptions): Promise<{
    evaluateInContext: (filepath: string) => void;
    runtime: EdgeRuntime;
    paths: Map<string, string>;
    warnedEvals: Set<string>;
}>;
export {};
