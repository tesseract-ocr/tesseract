/**
 * Removes the trailing slash for a given route or page path. Preserves the
 * root page. Examples:
 *   - `/foo/bar/` -> `/foo/bar`
 *   - `/foo/bar` -> `/foo/bar`
 *   - `/` -> `/`
 */ export function removeTrailingSlash(route) {
    return route.replace(/\/$/, '') || '/';
}

//# sourceMappingURL=remove-trailing-slash.js.map