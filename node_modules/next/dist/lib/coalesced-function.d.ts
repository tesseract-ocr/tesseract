type CoalescedInvoke<T> = {
    isOrigin: boolean;
    value: T;
};
export type UnwrapPromise<T> = T extends Promise<infer U> ? U : T;
export declare function withCoalescedInvoke<F extends (...args: any) => any>(func: F): (key: string, args: Parameters<F>) => Promise<CoalescedInvoke<UnwrapPromise<ReturnType<F>>>>;
export {};
