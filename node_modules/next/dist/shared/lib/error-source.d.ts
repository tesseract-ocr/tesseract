export declare function getErrorSource(error: Error): 'server' | 'edge-server' | null;
export type ErrorSourceType = 'edge-server' | 'server';
export declare function decorateServerError(error: Error, type: ErrorSourceType): void;
