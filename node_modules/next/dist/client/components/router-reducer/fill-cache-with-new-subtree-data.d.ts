import type { CacheNode } from '../../../shared/lib/app-router-context.shared-runtime';
import type { PrefetchCacheEntry } from './router-reducer-types';
import type { NormalizedFlightData } from '../../flight-data-helpers';
/**
 * Fill cache with rsc based on flightDataPath
 */
export declare function fillCacheWithNewSubTreeData(newCache: CacheNode, existingCache: CacheNode, flightData: NormalizedFlightData, prefetchEntry?: PrefetchCacheEntry): void;
export declare function fillCacheWithNewSubTreeDataButOnlyLoading(newCache: CacheNode, existingCache: CacheNode, flightData: NormalizedFlightData, prefetchEntry?: PrefetchCacheEntry): void;
