export declare function omit<T extends {
    [key: string]: unknown;
}, K extends keyof T>(object: T, keys: K[]): Omit<T, K>;
