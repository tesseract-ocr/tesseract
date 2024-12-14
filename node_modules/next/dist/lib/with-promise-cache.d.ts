import type { LRUCache } from '../server/lib/lru-cache';
export declare function withPromiseCache<K, V>(cache: LRUCache<Promise<V>>, fn: (value: K) => Promise<V>): (value: K) => Promise<V>;
export declare function withPromiseCache<T extends any[], K, V>(cache: LRUCache<Promise<V>>, fn: (...values: T) => Promise<V>, getKey: (...values: T) => K): (...values: T) => Promise<V>;
