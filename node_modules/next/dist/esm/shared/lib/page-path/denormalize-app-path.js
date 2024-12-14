export function denormalizeAppPagePath(page) {
    // `/` is normalized to `/index`
    if (page === '/index') {
        return '/';
    }
    return page;
}

//# sourceMappingURL=denormalize-app-path.js.map