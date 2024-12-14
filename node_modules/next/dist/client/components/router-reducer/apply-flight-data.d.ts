import type { CacheNode } from '../../../shared/lib/app-router-context.shared-runtime';
import type { PrefetchCacheEntry } from './router-reducer-types';
import type { NormalizedFlightData } from '../../flight-data-helpers';
export declare function applyFlightData(existingCache: CacheNode, cache: CacheNode, flightData: NormalizedFlightData, prefetchEntry?: PrefetchCacheEntry): boolean;
