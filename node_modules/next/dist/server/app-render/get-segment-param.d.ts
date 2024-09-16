import type { DynamicParamTypes } from './types';
/**
 * Parse dynamic route segment to type of parameter
 */
export declare function getSegmentParam(segment: string): {
    param: string;
    type: DynamicParamTypes;
} | null;
