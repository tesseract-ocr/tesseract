import type { ActionManifest } from '../../build/webpack/plugins/flight-client-entry-plugin';
import type { ClientReferenceManifest } from '../../build/webpack/plugins/flight-manifest-plugin';
import type { DeepReadonly } from '../../shared/lib/deep-readonly';
export declare function arrayBufferToString(buffer: ArrayBuffer): string;
export declare function stringToUint8Array(binary: string): Uint8Array;
export declare function encrypt(key: CryptoKey, iv: Uint8Array, data: Uint8Array): Promise<ArrayBuffer>;
export declare function decrypt(key: CryptoKey, iv: Uint8Array, data: Uint8Array): Promise<ArrayBuffer>;
export declare function generateEncryptionKeyBase64(dev?: boolean): Promise<string>;
export declare function setReferenceManifestsSingleton({ clientReferenceManifest, serverActionsManifest, serverModuleMap, }: {
    clientReferenceManifest: DeepReadonly<ClientReferenceManifest>;
    serverActionsManifest: DeepReadonly<ActionManifest>;
    serverModuleMap: {
        [id: string]: {
            id: string;
            chunks: string[];
            name: string;
        };
    };
}): void;
export declare function getServerModuleMap(): {
    [id: string]: {
        id: string;
        chunks: string[];
        name: string;
    };
};
export declare function getClientReferenceManifestSingleton(): {
    readonly moduleLoading: {
        readonly prefix: string;
        readonly crossOrigin: string | null;
    };
    readonly clientModules: {
        readonly [x: string]: {
            readonly id: string | number;
            readonly name: string;
            readonly chunks: readonly string[];
            readonly async?: boolean | undefined;
        };
    };
    readonly ssrModuleMapping: {
        readonly [x: string]: {
            readonly [x: string]: {
                readonly id: string | number;
                readonly name: string;
                readonly chunks: readonly string[];
                readonly async?: boolean | undefined;
            };
        };
    };
    readonly edgeSSRModuleMapping: {
        readonly [x: string]: {
            readonly [x: string]: {
                readonly id: string | number;
                readonly name: string;
                readonly chunks: readonly string[];
                readonly async?: boolean | undefined;
            };
        };
    };
    readonly entryCSSFiles: {
        readonly [x: string]: readonly string[];
    };
    readonly entryJSFiles?: {
        readonly [x: string]: readonly string[];
    } | undefined;
};
export declare function getActionEncryptionKey(): Promise<CryptoKey>;
