export function hasLoadingComponentInTree(tree) {
    const [, parallelRoutes, { loading }] = tree;
    if (loading) {
        return true;
    }
    return Object.values(parallelRoutes).some((parallelRoute)=>hasLoadingComponentInTree(parallelRoute));
}

//# sourceMappingURL=has-loading-component-in-tree.js.map