/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
import { webpack } from 'next/dist/compiled/webpack/webpack';
interface Options {
    dev: boolean;
    appDir: string;
}
/**
 * Webpack module id
 */
type ModuleId = string | number;
export type ManifestChunks = Array<string>;
export interface ManifestNode {
    [moduleExport: string]: {
        /**
         * Webpack module id
         */
        id: ModuleId;
        /**
         * Export name
         */
        name: string;
        /**
         * Chunks for the module. JS and CSS.
         */
        chunks: ManifestChunks;
        /**
         * If chunk contains async module
         */
        async?: boolean;
    };
}
export type ClientReferenceManifest = {
    readonly moduleLoading: {
        prefix: string;
        crossOrigin: string | null;
    };
    clientModules: ManifestNode;
    ssrModuleMapping: {
        [moduleId: string]: ManifestNode;
    };
    edgeSSRModuleMapping: {
        [moduleId: string]: ManifestNode;
    };
    entryCSSFiles: {
        [entry: string]: string[];
    };
    entryJSFiles?: {
        [entry: string]: string[];
    };
};
export declare class ClientReferenceManifestPlugin {
    dev: Options['dev'];
    appDir: Options['appDir'];
    appDirBase: string;
    constructor(options: Options);
    apply(compiler: webpack.Compiler): void;
    createAsset(assets: webpack.Compilation['assets'], compilation: webpack.Compilation, context: string): void;
}
export {};
