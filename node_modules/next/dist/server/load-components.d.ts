import type { AppType, DocumentType, NextComponentType } from '../shared/lib/utils';
import type { ClientReferenceManifest } from '../build/webpack/plugins/flight-manifest-plugin';
import type { PageConfig, GetStaticPaths, GetServerSideProps, GetStaticProps } from 'next/types';
import type { RouteModule } from './future/route-modules/route-module';
import type { BuildManifest } from './get-page-files';
import type { DeepReadonly } from '../shared/lib/deep-readonly';
export type ManifestItem = {
    id: number | string;
    files: string[];
};
export type ReactLoadableManifest = {
    [moduleId: string]: ManifestItem;
};
/**
 * A manifest entry type for the react-loadable-manifest.json.
 *
 * The whole manifest.json is a type of `Record<pathName, LoadableManifest>`
 * where pathName is a string-based key points to the path of the page contains
 * each dynamic imports.
 */
export interface LoadableManifest {
    [k: string]: {
        id: string | number;
        files: string[];
    };
}
export type LoadComponentsReturnType<NextModule = any> = {
    Component: NextComponentType;
    pageConfig: PageConfig;
    buildManifest: DeepReadonly<BuildManifest>;
    subresourceIntegrityManifest?: DeepReadonly<Record<string, string>>;
    reactLoadableManifest: DeepReadonly<ReactLoadableManifest>;
    clientReferenceManifest?: DeepReadonly<ClientReferenceManifest>;
    serverActionsManifest?: any;
    Document: DocumentType;
    App: AppType;
    getStaticProps?: GetStaticProps;
    getStaticPaths?: GetStaticPaths;
    getServerSideProps?: GetServerSideProps;
    ComponentMod: NextModule;
    routeModule?: RouteModule;
    isAppPath?: boolean;
    page: string;
    multiZoneDraftMode?: boolean;
};
/**
 * Load manifest file with retries, defaults to 3 attempts.
 */
export declare function loadManifestWithRetries<T extends object>(manifestPath: string, attempts?: number): Promise<DeepReadonly<T>>;
/**
 * Load manifest file with retries, defaults to 3 attempts.
 */
export declare function evalManifestWithRetries<T extends object>(manifestPath: string, attempts?: number): Promise<DeepReadonly<T>>;
declare function loadComponentsImpl<N = any>({ distDir, page, isAppPath, }: {
    distDir: string;
    page: string;
    isAppPath: boolean;
}): Promise<LoadComponentsReturnType<N>>;
export declare const loadComponents: typeof loadComponentsImpl;
export {};
