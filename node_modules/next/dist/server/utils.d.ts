export declare function isBlockedPage(page: string): boolean;
export declare function cleanAmpPath(pathname: string): string;
type AnyFunc<T> = (this: T, ...args: any) => any;
export declare function debounce<T, F extends AnyFunc<T>>(fn: F, ms: number, maxWait?: number): (this: T, ...passedArgs: Parameters<F>) => void;
export {};
