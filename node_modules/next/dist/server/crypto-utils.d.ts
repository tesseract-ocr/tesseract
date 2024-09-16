/// <reference types="node" />
export declare function encryptWithSecret(secret: Buffer, data: string): string;
export declare function decryptWithSecret(secret: Buffer, encryptedData: string): string;
