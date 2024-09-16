import type { FlightSegmentPath } from '../../../server/app-render/types';
import type { CacheNode } from '../../../shared/lib/app-router-context.shared-runtime';
/**
 * This will clear the CacheNode data for a particular segment path. This will cause a lazy-fetch in layout router to fill in new data.
 */
export declare function clearCacheNodeDataForSegmentPath(newCache: CacheNode, existingCache: CacheNode, flightSegmentPath: FlightSegmentPath): void;
