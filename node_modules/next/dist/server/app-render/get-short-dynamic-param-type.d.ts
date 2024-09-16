import type { DynamicParamTypes, DynamicParamTypesShort } from './types';
export declare const dynamicParamTypes: Record<DynamicParamTypes, DynamicParamTypesShort>;
/**
 * Shorten the dynamic param in order to make it smaller when transmitted to the browser.
 */
export declare function getShortDynamicParamType(type: DynamicParamTypes): DynamicParamTypesShort;
