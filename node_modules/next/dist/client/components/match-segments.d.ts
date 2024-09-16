import type { Segment } from '../../server/app-render/types';
export declare const matchSegment: (existingSegment: Segment, segment: Segment) => boolean;
export declare const canSegmentBeOverridden: (existingSegment: Segment, segment: Segment) => boolean;
