import { IncrementalCacheKind, type IncrementalCacheItem, type ResponseCacheEntry } from './types';
import { RouteKind } from '../route-kind';
export declare function fromResponseCacheEntry(cacheEntry: ResponseCacheEntry): Promise<IncrementalCacheItem>;
export declare function toResponseCacheEntry(response: IncrementalCacheItem): Promise<ResponseCacheEntry | null>;
export declare function routeKindToIncrementalCacheKind(routeKind: RouteKind): IncrementalCacheKind;
