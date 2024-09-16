import type { CacheNode } from '../../../shared/lib/app-router-context.shared-runtime';
import type { FlightDataPath } from '../../../server/app-render/types';
import type { PrefetchCacheEntry } from './router-reducer-types';
/**
 * Fill cache with rsc based on flightDataPath
 */
export declare function fillCacheWithNewSubTreeData(newCache: CacheNode, existingCache: CacheNode, flightDataPath: FlightDataPath, prefetchEntry?: PrefetchCacheEntry): void;
