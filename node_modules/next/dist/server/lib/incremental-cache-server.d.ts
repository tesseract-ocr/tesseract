import { IncrementalCache } from './incremental-cache';
declare let initializeResult: undefined | {
    ipcPort: number;
    ipcValidationKey: string;
};
export declare function initialize(...constructorArgs: ConstructorParameters<typeof IncrementalCache>): Promise<NonNullable<typeof initializeResult>>;
export {};
