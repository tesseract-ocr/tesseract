/**
 * For a given page path, this function ensures that there is no backslash
 * escaping slashes in the path. Example:
 *  - `foo\/bar\/baz` -> `foo/bar/baz`
 */ export function normalizePathSep(path) {
    return path.replace(/\\/g, '/');
}

//# sourceMappingURL=normalize-path-sep.js.map