import type { CacheNode } from '../../../shared/lib/app-router-context.shared-runtime';
import type { FlightRouterState, CacheNodeSeedData } from '../../../server/app-render/types';
import { type PrefetchCacheEntry } from './router-reducer-types';
export declare function fillLazyItemsTillLeafWithHead(newCache: CacheNode, existingCache: CacheNode | undefined, routerState: FlightRouterState, cacheNodeSeedData: CacheNodeSeedData | null, head: React.ReactNode, prefetchEntry?: PrefetchCacheEntry): void;
