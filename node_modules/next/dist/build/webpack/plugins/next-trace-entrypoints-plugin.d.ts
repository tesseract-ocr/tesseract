import type { Span } from '../../../trace';
import type { NodeFileTraceReasons } from 'next/dist/compiled/@vercel/nft';
import { webpack } from 'next/dist/compiled/webpack/webpack';
import type { NextConfigComplete } from '../../../server/config-shared';
export declare const TRACE_IGNORES: string[];
export declare function getFilesMapFromReasons(fileList: Set<string>, reasons: NodeFileTraceReasons, ignoreFn?: (file: string, parent?: string) => Boolean): Map<string, Set<string>>;
export interface TurbotraceAction {
    action: 'print' | 'annotate';
    input: string[];
    contextDirectory: string;
    processCwd: string;
    logLevel?: NonNullable<NextConfigComplete['experimental']['turbotrace']>['logLevel'];
    showAll?: boolean;
    memoryLimit?: number;
}
export interface BuildTraceContext {
    entriesTrace?: {
        action: TurbotraceAction;
        appDir: string;
        outputPath: string;
        depModArray: string[];
        entryNameMap: Record<string, string>;
    };
    chunksTrace?: {
        action: TurbotraceAction;
        outputPath: string;
        entryNameFilesMap: Record<string, Array<string>>;
    };
}
export declare class TraceEntryPointsPlugin implements webpack.WebpackPluginInstance {
    buildTraceContext: BuildTraceContext;
    private rootDir;
    private appDir;
    private pagesDir;
    private optOutBundlingPackages;
    private appDirEnabled?;
    private tracingRoot;
    private entryTraces;
    private traceIgnores;
    private esmExternals?;
    private turbotrace?;
    constructor({ rootDir, appDir, pagesDir, optOutBundlingPackages, appDirEnabled, traceIgnores, esmExternals, outputFileTracingRoot, turbotrace, }: {
        rootDir: string;
        appDir: string | undefined;
        pagesDir: string | undefined;
        optOutBundlingPackages: string[];
        appDirEnabled?: boolean;
        traceIgnores?: string[];
        outputFileTracingRoot?: string;
        esmExternals?: NextConfigComplete['experimental']['esmExternals'];
        turbotrace?: NextConfigComplete['experimental']['turbotrace'];
    });
    createTraceAssets(compilation: any, assets: any, span: Span): Promise<void>;
    tapfinishModules(compilation: webpack.Compilation, traceEntrypointsPluginSpan: Span, doResolve: (request: string, parent: string, job: import('@vercel/nft/out/node-file-trace').Job, isEsmRequested: boolean) => Promise<string>, readlink: any, stat: any): void;
    apply(compiler: webpack.Compiler): void;
}
