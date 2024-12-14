import type { Segment } from '../../server/app-render/types';
export declare function isGroupSegment(segment: string): boolean;
export declare function isParallelRouteSegment(segment: string): boolean;
export declare function addSearchParamsIfPageSegment(segment: Segment, searchParams: Record<string, string | string[] | undefined>): string | [string, string, "d" | "c" | "ci" | "oc" | "di"];
export declare const PAGE_SEGMENT_KEY = "__PAGE__";
export declare const DEFAULT_SEGMENT_KEY = "__DEFAULT__";
