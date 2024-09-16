declare abstract class ResourceManager<T, K> {
    private resources;
    abstract create(resourceArgs: K): T;
    abstract destroy(resource: T): void;
    add(resourceArgs: K): T;
    remove(resource: T): void;
    removeAll(): void;
}
declare class IntervalsManager extends ResourceManager<number, Parameters<typeof setInterval>> {
    create(args: Parameters<typeof setInterval>): number;
    destroy(interval: number): void;
}
declare class TimeoutsManager extends ResourceManager<number, Parameters<typeof setTimeout>> {
    create(args: Parameters<typeof setTimeout>): number;
    destroy(timeout: number): void;
}
export declare const intervalsManager: IntervalsManager;
export declare const timeoutsManager: TimeoutsManager;
export {};
