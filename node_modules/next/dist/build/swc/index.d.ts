import { type DefineEnvPluginOptions } from '../webpack/plugins/define-env-plugin';
import type { Binding, DefineEnv } from './types';
/**
 * Based on napi-rs's target triples, returns triples that have corresponding next-swc binaries.
 */
export declare function getSupportedArchTriples(): Record<string, any>;
export declare const lockfilePatchPromise: {
    cur?: Promise<void>;
};
export declare function loadBindings(useWasmBinary?: boolean): Promise<Binding>;
export declare function createDefineEnv({ isTurbopack, clientRouterFilters, config, dev, distDir, fetchCacheKeyPrefix, hasRewrites, middlewareMatchers, }: Omit<DefineEnvPluginOptions, 'isClient' | 'isNodeOrEdgeCompilation' | 'isEdgeServer' | 'isNodeServer'>): DefineEnv;
export declare function isWasm(): Promise<boolean>;
export declare function transform(src: string, options?: any): Promise<any>;
export declare function transformSync(src: string, options?: any): any;
export declare function minify(src: string, options: any): Promise<string>;
export declare function parse(src: string, options: any): Promise<any>;
export declare function getBinaryMetadata(): {
    target: string | undefined;
};
/**
 * Initialize trace subscriber to emit traces.
 *
 */
export declare function initCustomTraceSubscriber(traceFileName?: string): void;
/**
 * Initialize heap profiler, if possible.
 * Note this is not available in release build of next-swc by default,
 * only available by manually building next-swc with specific flags.
 * Calling in release build will not do anything.
 */
export declare function initHeapProfiler(): void;
/**
 * Teardown heap profiler, if possible.
 *
 * Same as initialization, this is not available in release build of next-swc by default
 * and calling it will not do anything.
 */
export declare const teardownHeapProfiler: () => void;
/**
 * Teardown swc's trace subscriber if there's an initialized flush guard exists.
 *
 * This is workaround to amend behavior with process.exit
 * (https://github.com/vercel/next.js/blob/4db8c49cc31e4fc182391fae6903fb5ef4e8c66e/packages/next/bin/next.ts#L134=)
 * seems preventing napi's cleanup hook execution (https://github.com/swc-project/swc/blob/main/crates/node/src/util.rs#L48-L51=),
 *
 * instead parent process manually drops guard when process gets signal to exit.
 */
export declare const teardownTraceSubscriber: () => void;
