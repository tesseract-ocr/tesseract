import type { Manifest, ManifestLoader } from './manifest-loader';
export declare class ServerManifestLoader implements ManifestLoader {
    private readonly getter;
    constructor(getter: (name: string) => Manifest | null);
    load(name: string): Manifest | null;
}
