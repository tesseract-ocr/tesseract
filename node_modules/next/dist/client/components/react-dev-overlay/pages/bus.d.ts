import type { BusEvent } from '../shared';
export type BusEventHandler = (ev: BusEvent) => void;
export declare function emit(ev: BusEvent): void;
export declare function on(fn: BusEventHandler): boolean;
export declare function off(fn: BusEventHandler): boolean;
