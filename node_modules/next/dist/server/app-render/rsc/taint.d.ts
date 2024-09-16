type Reference = object;
type TaintableUniqueValue = string | bigint | ArrayBufferView;
export declare const taintObjectReference: (message: string | undefined, object: Reference) => void;
export declare const taintUniqueValue: (message: string | undefined, lifetime: Reference, value: TaintableUniqueValue) => void;
export {};
