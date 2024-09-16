import type { IncrementalCacheItem, ResponseCacheEntry } from './types';
export declare function fromResponseCacheEntry(cacheEntry: ResponseCacheEntry): Promise<IncrementalCacheItem>;
export declare function toResponseCacheEntry(response: IncrementalCacheItem): Promise<ResponseCacheEntry | null>;
