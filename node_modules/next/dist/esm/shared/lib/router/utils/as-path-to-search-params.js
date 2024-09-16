// Convert router.asPath to a URLSearchParams object
// example: /dynamic/[slug]?foo=bar -> { foo: 'bar' }
export function asPathToSearchParams(asPath) {
    return new URL(asPath, "http://n").searchParams;
}

//# sourceMappingURL=as-path-to-search-params.js.map