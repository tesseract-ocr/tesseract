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
            [name: string]: string | number;
        };
        layer: {
            [name: string]: string;
        };
    };
};
export type ActionManifest = {
    encryptionKey: string;
    node: Actions;
    edge: Actions;
};
export declare class FlightClientEntryPlugin {
    dev: boolean;
    appDir: string;
    encryptionKey: string;
    isEdgeServer: boolean;
    assetPrefix: string;
    constructor(options: Options);
    apply(compiler: webpack.Compiler): void;
    createClientEntries(compiler: webpack.Compiler, compilation: webpack.Compilation): Promise<void>;
    collectClientActionsFromDependencies({ compilation, dependencies, }: {
        compilation: webpack.Compilation;
        dependencies: ReturnType<typeof webpack.EntryPlugin.createDependency>[];
    }): Map<string, string[]>;
    collectComponentInfoFromServerEntryDependency({ entryRequest, compilation, resolvedModule, }: {
        entryRequest: string;
        compilation: webpack.Compilation;
        resolvedModule: any;
    }): {
        cssImports: CssImports;
        clientComponentImports: ClientComponentImports;
        actionImports: [string, string[]][];
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
        addEntryPromise: Promise<void>,
        ssrDep: ReturnType<typeof webpack.EntryPlugin.createDependency>
    ];
    injectActionEntry({ compiler, compilation, actions, entryName, bundlePath, fromClient, }: {
        compiler: webpack.Compiler;
        compilation: webpack.Compilation;
        actions: Map<string, string[]>;
        entryName: string;
        bundlePath: string;
        fromClient?: boolean;
    }): Promise<any>;
    addEntry(compilation: any, context: string, dependency: webpack.Dependency, options: webpack.EntryOptions): Promise<any>;
    createActionAssets(compilation: webpack.Compilation, assets: webpack.Compilation['assets']): Promise<void>;
}
export {};
