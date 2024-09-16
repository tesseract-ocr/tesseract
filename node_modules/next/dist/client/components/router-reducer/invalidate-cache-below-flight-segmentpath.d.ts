import type { CacheNode } from '../../../shared/lib/app-router-context.shared-runtime';
import type { FlightSegmentPath } from '../../../server/app-render/types';
/**
 * Fill cache up to the end of the flightSegmentPath, invalidating anything below it.
 */
export declare function invalidateCacheBelowFlightSegmentPath(newCache: CacheNode, existingCache: CacheNode, flightSegmentPath: FlightSegmentPath): void;
