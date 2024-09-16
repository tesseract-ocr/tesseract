export declare class PromiseQueue {
    #private;
    constructor(maxConcurrency?: number);
    enqueue<T>(promiseFn: () => Promise<T>): Promise<T>;
    bump(promiseFn: Promise<any>): void;
}
