import type { NodejsRequestData, FetchEventResult } from '../types';
import type { EdgeFunctionDefinition } from '../../../build/webpack/plugins/middleware-plugin';
import type { EdgeRuntime } from 'next/dist/compiled/edge-runtime';
export declare const ErrorSource: unique symbol;
type RunnerFn = (params: {
    name: string;
    onError?: (err: unknown) => void;
    onWarning?: (warn: Error) => void;
    paths: string[];
    request: NodejsRequestData;
    useCache: boolean;
    edgeFunctionEntry: Pick<EdgeFunctionDefinition, 'wasm' | 'assets'>;
    distDir: string;
    incrementalCache?: any;
}) => Promise<FetchEventResult>;
export declare function getRuntimeContext(params: {
    name: string;
    onWarning?: any;
    onError?: (err: unknown) => void;
    useCache: boolean;
    edgeFunctionEntry: any;
    distDir: string;
    paths: string[];
    incrementalCache?: any;
}): Promise<EdgeRuntime<any>>;
export declare const run: RunnerFn;
export {};
