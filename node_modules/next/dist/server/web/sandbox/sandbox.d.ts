import type { NodejsRequestData, FetchEventResult } from '../types';
import type { EdgeFunctionDefinition } from '../../../build/webpack/plugins/middleware-plugin';
import type { EdgeRuntime } from 'next/dist/compiled/edge-runtime';
import type { ServerComponentsHmrCache } from '../../response-cache';
export declare const ErrorSource: unique symbol;
interface RunnerFnParams {
    name: string;
    onError?: (err: unknown) => void;
    onWarning?: (warn: Error) => void;
    paths: string[];
    request: NodejsRequestData;
    useCache: boolean;
    edgeFunctionEntry: Pick<EdgeFunctionDefinition, 'assets' | 'wasm' | 'env'>;
    distDir: string;
    incrementalCache?: any;
    serverComponentsHmrCache?: ServerComponentsHmrCache;
}
type RunnerFn = (params: RunnerFnParams) => Promise<FetchEventResult>;
export declare function getRuntimeContext(params: Omit<RunnerFnParams, 'request'>): Promise<EdgeRuntime<any>>;
export declare const run: RunnerFn;
export {};
