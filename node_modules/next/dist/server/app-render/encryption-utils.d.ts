import type { ActionManifest } from '../../build/webpack/plugins/flight-client-entry-plugin';
import type { ClientReferenceManifest, ClientReferenceManifestForRsc } from '../../build/webpack/plugins/flight-manifest-plugin';
import type { DeepReadonly } from '../../shared/lib/deep-readonly';
export declare function arrayBufferToString(buffer: ArrayBuffer | Uint8Array<ArrayBufferLike>): string;
export declare function stringToUint8Array(binary: string): Uint8Array<ArrayBuffer>;
export declare function encrypt(key: CryptoKey, iv: Uint8Array, data: Uint8Array): Promise<ArrayBuffer>;
export declare function decrypt(key: CryptoKey, iv: Uint8Array, data: Uint8Array): Promise<ArrayBuffer>;
export declare function setReferenceManifestsSingleton({ page, clientReferenceManifest, serverActionsManifest, serverModuleMap, }: {
    page: string;
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
export declare function getClientReferenceManifestForRsc(): DeepReadonly<ClientReferenceManifestForRsc>;
export declare function getActionEncryptionKey(): Promise<CryptoKey>;
