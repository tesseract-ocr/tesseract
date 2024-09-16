import type { Addresses, EnvVars, FS, SerializableTurborepoAccessTraceResult, PublicTurborepoAccessTraceResult } from './types';
export declare class TurborepoAccessTraceResult {
    private envVars;
    private addresses;
    private fsPaths;
    constructor(envVars?: EnvVars, addresses?: Addresses, fsPaths?: FS);
    /**
     * Merge another `TurborepoAccessTraceResult` into this one, mutating this `TurborepoAccessTraceResult`.
     */
    merge(other: TurborepoAccessTraceResult): this;
    /**
     * Serialize this `TurborepoAccessTraceResult` into a serializable object. Used for passing
     * the `TurborepoAccessTraceResult` between workers where Sets are not serializable.
     */
    serialize(): SerializableTurborepoAccessTraceResult;
    /**
     * Squash this `TurborepoAccessTraceResult` into a public trace object that can be written to a file
     */
    toPublicTrace(): PublicTurborepoAccessTraceResult;
    /**
     * Create an `TurborepoAccessTraceResult` from a serialized `SerializableTurborepoAccessTraceResult`
     */
    static fromSerialized(serialized: SerializableTurborepoAccessTraceResult): TurborepoAccessTraceResult;
}
