/**
 * `app` -> app dir
 * `pages` -> pages dir
 * `root` -> middleware / instrumentation
 * `assets` -> assets
 */
export type EntryKeyType = 'app' | 'pages' | 'root' | 'assets';
export type EntryKeySide = 'client' | 'server';
export type EntryKey = `{"type":"${EntryKeyType}","side":"${EntryKeyType}","page":"${string}"}`;
/**
 * Get a key that's unique across all entrypoints.
 */
export declare function getEntryKey(type: EntryKeyType, side: EntryKeySide, page: string): EntryKey;
/**
 * Split an `EntryKey` up into its components.
 */
export declare function splitEntryKey(key: EntryKey): {
    type: EntryKeyType;
    side: EntryKeySide;
    page: string;
};
