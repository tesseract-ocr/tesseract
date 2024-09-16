import type { NextConfigComplete } from '../../server/config-shared';
import type { DefineEnvPluginOptions } from '../webpack/plugins/define-env-plugin';
import type { __ApiPreviewProps } from '../../server/api-utils';
/**
 * Based on napi-rs's target triples, returns triples that have corresponding next-swc binaries.
 */
export declare const getSupportedArchTriples: () => Record<string, any>;
export declare const lockfilePatchPromise: {
    cur?: Promise<void>;
};
export interface Binding {
    isWasm: boolean;
    turbo: {
        startTrace: any;
        nextBuild?: any;
        createTurboTasks?: any;
        entrypoints: {
            stream: any;
            get: any;
        };
        mdx: {
            compile: any;
            compileSync: any;
        };
        createProject: (options: ProjectOptions, turboEngineOptions?: TurboEngineOptions) => Promise<Project>;
    };
    minify: any;
    minifySync: any;
    transform: any;
    transformSync: any;
    parse: any;
    parseSync: any;
    getTargetTriple(): string | undefined;
    initCustomTraceSubscriber?: any;
    teardownTraceSubscriber?: any;
    initHeapProfiler?: any;
    teardownHeapProfiler?: any;
    css: {
        lightning: {
            transform(transformOptions: any): Promise<any>;
            transformStyleAttr(attrOptions: any): Promise<any>;
        };
    };
}
export declare function loadBindings(useWasmBinary?: boolean): Promise<Binding>;
export interface ProjectOptions {
    /**
     * A root path from which all files must be nested under. Trying to access
     * a file outside this root will fail. Think of this as a chroot.
     */
    rootPath: string;
    /**
     * A path inside the root_path which contains the app/pages directories.
     */
    projectPath: string;
    /**
     * The next.config.js contents.
     */
    nextConfig: NextConfigComplete;
    /**
     * Jsconfig, or tsconfig contents.
     *
     * Next.js implicitly requires to read it to support few options
     * https://nextjs.org/docs/architecture/nextjs-compiler#legacy-decorators
     * https://nextjs.org/docs/architecture/nextjs-compiler#importsource
     */
    jsConfig: {
        compilerOptions: object;
    };
    /**
     * A map of environment variables to use when compiling code.
     */
    env: Record<string, string>;
    defineEnv: DefineEnv;
    /**
     * Whether to watch the filesystem for file changes.
     */
    watch: boolean;
    /**
     * The mode in which Next.js is running.
     */
    dev: boolean;
    /**
     * The server actions encryption key.
     */
    encryptionKey: string;
    /**
     * The build id.
     */
    buildId: string;
    /**
     * Options for draft mode.
     */
    previewProps: __ApiPreviewProps;
}
type RustifiedEnv = {
    name: string;
    value: string;
}[];
export interface DefineEnv {
    client: RustifiedEnv;
    edge: RustifiedEnv;
    nodejs: RustifiedEnv;
}
export declare function createDefineEnv({ isTurbopack, clientRouterFilters, config, dev, distDir, fetchCacheKeyPrefix, hasRewrites, middlewareMatchers, }: Omit<DefineEnvPluginOptions, 'isClient' | 'isNodeOrEdgeCompilation' | 'isEdgeServer' | 'isNodeServer'>): DefineEnv;
export interface TurboEngineOptions {
    /**
     * An upper bound of memory that turbopack will attempt to stay under.
     */
    memoryLimit?: number;
}
export type StyledString = {
    type: 'text';
    value: string;
} | {
    type: 'code';
    value: string;
} | {
    type: 'strong';
    value: string;
} | {
    type: 'stack';
    value: StyledString[];
} | {
    type: 'line';
    value: StyledString[];
};
export interface Issue {
    severity: string;
    stage: string;
    filePath: string;
    title: StyledString;
    description?: StyledString;
    detail?: StyledString;
    source?: {
        source: {
            ident: string;
            content?: string;
        };
        range?: {
            start: {
                line: number;
                column: number;
            };
            end: {
                line: number;
                column: number;
            };
        };
    };
    documentationLink: string;
    subIssues: Issue[];
}
export interface Diagnostics {
    category: string;
    name: string;
    payload: unknown;
}
export type TurbopackResult<T = {}> = T & {
    issues: Issue[];
    diagnostics: Diagnostics[];
};
export interface Middleware {
    endpoint: Endpoint;
}
export interface Instrumentation {
    nodeJs: Endpoint;
    edge: Endpoint;
}
export interface Entrypoints {
    routes: Map<string, Route>;
    middleware?: Middleware;
    instrumentation?: Instrumentation;
    pagesDocumentEndpoint: Endpoint;
    pagesAppEndpoint: Endpoint;
    pagesErrorEndpoint: Endpoint;
}
interface BaseUpdate {
    resource: {
        headers: unknown;
        path: string;
    };
    diagnostics: unknown[];
    issues: Issue[];
}
interface IssuesUpdate extends BaseUpdate {
    type: 'issues';
}
interface EcmascriptMergedUpdate {
    type: 'EcmascriptMergedUpdate';
    chunks: {
        [moduleName: string]: {
            type: 'partial';
        };
    };
    entries: {
        [moduleName: string]: {
            code: string;
            map: string;
            url: string;
        };
    };
}
interface PartialUpdate extends BaseUpdate {
    type: 'partial';
    instruction: {
        type: 'ChunkListUpdate';
        merged: EcmascriptMergedUpdate[] | undefined;
    };
}
export type Update = IssuesUpdate | PartialUpdate;
export interface HmrIdentifiers {
    identifiers: string[];
}
/** @see https://github.com/vercel/next.js/blob/415cd74b9a220b6f50da64da68c13043e9b02995/packages/next-swc/crates/napi/src/next_api/project.rs#L824-L833 */
export interface TurbopackStackFrame {
    isServer: boolean;
    isInternal?: boolean;
    file: string;
    /** 1-indexed, unlike source map tokens */
    line?: number;
    /** 1-indexed, unlike source map tokens */
    column?: number | null;
    methodName?: string;
}
export type UpdateMessage = {
    updateType: 'start';
} | {
    updateType: 'end';
    value: UpdateInfo;
};
export interface UpdateInfo {
    duration: number;
    tasks: number;
}
export interface Project {
    update(options: Partial<ProjectOptions>): Promise<void>;
    entrypointsSubscribe(): AsyncIterableIterator<TurbopackResult<Entrypoints>>;
    hmrEvents(identifier: string): AsyncIterableIterator<TurbopackResult<Update>>;
    hmrIdentifiersSubscribe(): AsyncIterableIterator<TurbopackResult<HmrIdentifiers>>;
    getSourceForAsset(filePath: string): Promise<string | null>;
    traceSource(stackFrame: TurbopackStackFrame): Promise<TurbopackStackFrame | null>;
    updateInfoSubscribe(aggregationMs: number): AsyncIterableIterator<TurbopackResult<UpdateMessage>>;
}
export type Route = {
    type: 'conflict';
} | {
    type: 'app-page';
    pages: {
        originalName: string;
        htmlEndpoint: Endpoint;
        rscEndpoint: Endpoint;
    }[];
} | {
    type: 'app-route';
    originalName: string;
    endpoint: Endpoint;
} | {
    type: 'page';
    htmlEndpoint: Endpoint;
    dataEndpoint: Endpoint;
} | {
    type: 'page-api';
    endpoint: Endpoint;
};
export interface Endpoint {
    /** Write files for the endpoint to disk. */
    writeToDisk(): Promise<TurbopackResult<WrittenEndpoint>>;
    /**
     * Listen to client-side changes to the endpoint.
     * After clientChanged() has been awaited it will listen to changes.
     * The async iterator will yield for each change.
     */
    clientChanged(): Promise<AsyncIterableIterator<TurbopackResult>>;
    /**
     * Listen to server-side changes to the endpoint.
     * After serverChanged() has been awaited it will listen to changes.
     * The async iterator will yield for each change.
     */
    serverChanged(includeIssues: boolean): Promise<AsyncIterableIterator<TurbopackResult>>;
}
interface EndpointConfig {
    dynamic?: 'auto' | 'force-dynamic' | 'error' | 'force-static';
    dynamicParams?: boolean;
    revalidate?: 'never' | 'force-cache' | number;
    fetchCache?: 'auto' | 'default-cache' | 'only-cache' | 'force-cache' | 'default-no-store' | 'only-no-store' | 'force-no-store';
    runtime?: 'nodejs' | 'edge';
    preferredRegion?: string;
}
export type ServerPath = {
    path: string;
    contentHash: string;
};
export type WrittenEndpoint = {
    type: 'nodejs';
    /** The entry path for the endpoint. */
    entryPath: string;
    /** All client paths that have been written for the endpoint. */
    clientPaths: string[];
    /** All server paths that have been written for the endpoint. */
    serverPaths: ServerPath[];
    config: EndpointConfig;
} | {
    type: 'edge';
    /** All client paths that have been written for the endpoint. */
    clientPaths: string[];
    /** All server paths that have been written for the endpoint. */
    serverPaths: ServerPath[];
    config: EndpointConfig;
};
export declare function isWasm(): Promise<boolean>;
export declare function transform(src: string, options?: any): Promise<any>;
export declare function transformSync(src: string, options?: any): any;
export declare function minify(src: string, options: any): Promise<string>;
export declare function minifySync(src: string, options: any): string;
export declare function parse(src: string, options: any): Promise<any>;
export declare function getBinaryMetadata(): {
    target: any;
};
/**
 * Initialize trace subscriber to emit traces.
 *
 */
export declare const initCustomTraceSubscriber: (traceFileName?: string) => void;
/**
 * Initialize heap profiler, if possible.
 * Note this is not available in release build of next-swc by default,
 * only available by manually building next-swc with specific flags.
 * Calling in release build will not do anything.
 */
export declare const initHeapProfiler: () => void;
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
export {};
