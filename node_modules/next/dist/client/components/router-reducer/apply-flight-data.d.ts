import type { CacheNode } from '../../../shared/lib/app-router-context.shared-runtime';
import type { FlightDataPath } from '../../../server/app-render/types';
import type { PrefetchCacheEntry } from './router-reducer-types';
export declare function applyFlightData(existingCache: CacheNode, cache: CacheNode, flightDataPath: FlightDataPath, prefetchEntry?: PrefetchCacheEntry): boolean;
