import type { CssImports, ClientComponentImports } from '../loaders/next-flight-client-entry-loader';
import { webpack } from 'next/dist/compiled/webpack/webpack';
interface Options {
    dev: boolean;
    appDir: string;
    isEdgeServer: boolean;
    encryptionKey: string;
}
type Actions = {
    [actionId: string]: {
        workers: {
            [name: string]: {
                moduleId: string | number;
                async: boolean;
            };
        };
        layer: {
            [name: string]: string;
        };
    };
};
type ActionIdNamePair = [id: string, name: string];
export type ActionManifest = {
    encryptionKey: string;
    node: Actions;
    edge: Actions;
};
export interface ModuleInfo {
    moduleId: string | number;
    async: boolean;
}
export declare class FlightClientEntryPlugin {
    dev: boolean;
    appDir: string;
    encryptionKey: string;
    isEdgeServer: boolean;
    assetPrefix: string;
    webpackRuntime: string;
    constructor(options: Options);
    apply(compiler: webpack.Compiler): void;
    createClientEntries(compiler: webpack.Compiler, compilation: webpack.Compilation): Promise<void>;
    collectClientActionsFromDependencies({ compilation, dependencies, }: {
        compilation: webpack.Compilation;
        dependencies: ReturnType<typeof webpack.EntryPlugin.createDependency>[];
    }): Map<string, ActionIdNamePair[]>;
    collectComponentInfoFromServerEntryDependency({ entryRequest, compilation, resolvedModule, }: {
        entryRequest: string;
        compilation: webpack.Compilation;
        resolvedModule: any;
    }): {
        cssImports: CssImports;
        clientComponentImports: ClientComponentImports;
        actionImports: [string, ActionIdNamePair[]][];
    };
    injectClientEntryAndSSRModules({ compiler, compilation, entryName, clientImports, bundlePath, absolutePagePath, }: {
        compiler: webpack.Compiler;
        compilation: webpack.Compilation;
        entryName: string;
        clientImports: ClientComponentImports;
        bundlePath: string;
        absolutePagePath?: string;
    }): [
        shouldInvalidate: boolean,
        addSSREntryPromise: Promise<void>,
        addRSCEntryPromise: Promise<void>,
        ssrDep: ReturnType<typeof webpack.EntryPlugin.createDependency>
    ];
    injectActionEntry({ compiler, compilation, actions, entryName, bundlePath, fromClient, createdActionIds, }: {
        compiler: webpack.Compiler;
        compilation: webpack.Compilation;
        actions: Map<string, ActionIdNamePair[]>;
        entryName: string;
        bundlePath: string;
        createdActionIds: Set<string>;
        fromClient?: boolean;
    }): Promise<any>;
    addEntry(compilation: any, context: string, dependency: webpack.Dependency, options: webpack.EntryOptions): Promise<any>;
    createActionAssets(compilation: webpack.Compilation, assets: webpack.Compilation['assets']): Promise<void>;
}
export {};
