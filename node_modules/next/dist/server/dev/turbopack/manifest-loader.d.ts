import type { MiddlewareManifest } from '../../../build/webpack/plugins/middleware-plugin';
import type { SetupOpts } from '../../lib/router-utils/setup-dev-bundler';
import type { PageEntrypoints } from './types';
import { type EntryKey } from './entry-key';
interface InstrumentationDefinition {
    files: string[];
    name: 'instrumentation';
}
type TurbopackMiddlewareManifest = MiddlewareManifest & {
    instrumentation?: InstrumentationDefinition;
};
export declare class TurbopackManifestLoader {
    private actionManifests;
    private appBuildManifests;
    private appPathsManifests;
    private buildManifests;
    private fontManifests;
    private loadableManifests;
    private middlewareManifests;
    private pagesManifests;
    private encryptionKey;
    private readonly distDir;
    private readonly buildId;
    constructor({ distDir, buildId, encryptionKey, }: {
        buildId: string;
        distDir: string;
        encryptionKey: string;
    });
    delete(key: EntryKey): void;
    loadActionManifest(pageName: string): Promise<void>;
    private mergeActionManifests;
    private writeActionManifest;
    loadAppBuildManifest(pageName: string): Promise<void>;
    private mergeAppBuildManifests;
    private writeAppBuildManifest;
    loadAppPathsManifest(pageName: string): Promise<void>;
    private writeAppPathsManifest;
    /**
     * Turbopack doesn't support this functionality, so it writes an empty manifest.
     */
    private writeAutomaticFontOptimizationManifest;
    loadBuildManifest(pageName: string, type?: 'app' | 'pages'): Promise<void>;
    private mergeBuildManifests;
    private writeBuildManifest;
    private writeFallbackBuildManifest;
    loadFontManifest(pageName: string, type?: 'app' | 'pages'): Promise<void>;
    private mergeFontManifests;
    private writeNextFontManifest;
    loadLoadableManifest(pageName: string, type?: 'app' | 'pages'): Promise<void>;
    private mergeLoadableManifests;
    private writeLoadableManifest;
    loadMiddlewareManifest(pageName: string, type: 'pages' | 'app' | 'middleware' | 'instrumentation'): Promise<void>;
    getMiddlewareManifest(key: EntryKey): TurbopackMiddlewareManifest | undefined;
    deleteMiddlewareManifest(key: EntryKey): boolean;
    private mergeMiddlewareManifests;
    private writeMiddlewareManifest;
    loadPagesManifest(pageName: string): Promise<void>;
    private mergePagesManifests;
    private writePagesManifest;
    writeManifests({ rewrites, pageEntrypoints, }: {
        rewrites: SetupOpts['fsChecker']['rewrites'];
        pageEntrypoints: PageEntrypoints;
    }): Promise<void>;
}
export {};
