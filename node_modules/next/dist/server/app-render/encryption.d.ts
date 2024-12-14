import 'server-only';
export declare function encryptActionBoundArgs(actionId: string, args: any[]): Promise<string>;
export declare function decryptActionBoundArgs(actionId: string, encrypted: Promise<string>): Promise<unknown>;
