interface Cache<K, V> {
    set(key: K, value: V, maxAge?: number): boolean;
    get(key: K): V | undefined;
    del(key: K): void;
}
export declare function withPromiseCache<K, V>(cache: Cache<K, Promise<V>>, fn: (value: K) => Promise<V>): (value: K) => Promise<V>;
export declare function withPromiseCache<T extends any[], K, V>(cache: Cache<K, Promise<V>>, fn: (...values: T) => Promise<V>, getKey: (...values: T) => K): (...values: T) => Promise<V>;
export {};
