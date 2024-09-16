/**
 * `app` -> app dir
 * `pages` -> pages dir
 * `root` -> middleware / instrumentation
 * `assets` -> assets
 */ /**
 * Get a key that's unique across all entrypoints.
 */ export function getEntryKey(type, side, page) {
    return JSON.stringify({
        type,
        side,
        page
    });
}
/**
 * Split an `EntryKey` up into its components.
 */ export function splitEntryKey(key) {
    return JSON.parse(key);
}

//# sourceMappingURL=entry-key.js.map