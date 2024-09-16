/**
 * Find the starting index of Uint8Array `b` within Uint8Array `a`.
 */
export declare function indexOfUint8Array(a: Uint8Array, b: Uint8Array): number;
/**
 * Check if two Uint8Arrays are strictly equivalent.
 */
export declare function isEquivalentUint8Arrays(a: Uint8Array, b: Uint8Array): boolean;
/**
 * Remove Uint8Array `b` from Uint8Array `a`.
 *
 * If `b` is not in `a`, `a` is returned unchanged.
 *
 * Otherwise, the function returns a new Uint8Array instance with size `a.length - b.length`
 */
export declare function removeFromUint8Array(a: Uint8Array, b: Uint8Array): Uint8Array;
