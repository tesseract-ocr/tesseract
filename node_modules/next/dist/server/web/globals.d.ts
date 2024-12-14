import type { InstrumentationModule, InstrumentationOnRequestError } from '../instrumentation/types';
export declare function getEdgeInstrumentationModule(): Promise<InstrumentationModule | undefined>;
export declare function edgeInstrumentationOnRequestError(...args: Parameters<InstrumentationOnRequestError>): Promise<void>;
export declare function ensureInstrumentationRegistered(): Promise<void>;
