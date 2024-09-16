"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "TurborepoAccessTraceResult", {
    enumerable: true,
    get: function() {
        return TurborepoAccessTraceResult;
    }
});
class TurborepoAccessTraceResult {
    constructor(envVars = new Set([]), addresses = [], fsPaths = new Set([])){
        this.envVars = envVars;
        this.addresses = addresses;
        this.fsPaths = fsPaths;
    }
    /**
   * Merge another `TurborepoAccessTraceResult` into this one, mutating this `TurborepoAccessTraceResult`.
   */ merge(other) {
        other.envVars.forEach((envVar)=>this.envVars.add(envVar));
        other.fsPaths.forEach((path)=>this.fsPaths.add(path));
        this.addresses.push(...other.addresses);
        return this;
    }
    /**
   * Serialize this `TurborepoAccessTraceResult` into a serializable object. Used for passing
   * the `TurborepoAccessTraceResult` between workers where Sets are not serializable.
   */ serialize() {
        return {
            fs: Array.from(this.fsPaths).map(String),
            addresses: this.addresses,
            envVars: Array.from(this.envVars).map(String)
        };
    }
    /**
   * Squash this `TurborepoAccessTraceResult` into a public trace object that can be written to a file
   */ toPublicTrace() {
        return {
            network: this.addresses.length > 0,
            envVarKeys: Array.from(this.envVars).map(String),
            filePaths: Array.from(this.fsPaths).map(String)
        };
    }
    /**
   * Create an `TurborepoAccessTraceResult` from a serialized `SerializableTurborepoAccessTraceResult`
   */ static fromSerialized(serialized) {
        return new TurborepoAccessTraceResult(new Set(serialized.envVars), serialized.addresses, new Set(serialized.fs));
    }
}

//# sourceMappingURL=result.js.map