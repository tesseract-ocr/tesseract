"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "hasLoadingComponentInTree", {
    enumerable: true,
    get: function() {
        return hasLoadingComponentInTree;
    }
});
function hasLoadingComponentInTree(tree) {
    const [, parallelRoutes, { loading }] = tree;
    if (loading) {
        return true;
    }
    return Object.values(parallelRoutes).some((parallelRoute)=>hasLoadingComponentInTree(parallelRoute));
}

//# sourceMappingURL=has-loading-component-in-tree.js.map