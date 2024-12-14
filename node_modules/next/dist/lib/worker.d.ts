import { Worker as JestWorker } from 'next/dist/compiled/jest-worker';
type FarmOptions = ConstructorParameters<typeof JestWorker>[1];
export declare class Worker {
    private _worker;
    constructor(workerPath: string, options: FarmOptions & {
        timeout?: number;
        onActivity?: () => void;
        onActivityAbort?: () => void;
        onRestart?: (method: string, args: any[], attempts: number) => void;
        logger?: Pick<typeof console, 'error' | 'info' | 'warn'>;
        exposedMethods: ReadonlyArray<string>;
        enableWorkerThreads?: boolean;
    });
    end(): ReturnType<JestWorker['end']>;
    /**
     * Quietly end the worker if it exists
     */
    close(): void;
}
export {};
