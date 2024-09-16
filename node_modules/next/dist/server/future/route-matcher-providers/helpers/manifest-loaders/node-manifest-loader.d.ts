import type { Manifest, ManifestLoader } from './manifest-loader';
export declare class NodeManifestLoader implements ManifestLoader {
    private readonly distDir;
    constructor(distDir: string);
    static require(id: string): any;
    load(name: string): Manifest | null;
}
