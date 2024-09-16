"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "parseLoaderTree", {
    enumerable: true,
    get: function() {
        return parseLoaderTree;
    }
});
const _segment = require("../../shared/lib/segment");
function parseLoaderTree(tree) {
    const [segment, parallelRoutes, components] = tree;
    const { layout } = components;
    let { page } = components;
    // a __DEFAULT__ segment means that this route didn't match any of the
    // segments in the route, so we should use the default page
    page = segment === _segment.DEFAULT_SEGMENT_KEY ? components.defaultPage : page;
    const layoutOrPagePath = (layout == null ? void 0 : layout[1]) || (page == null ? void 0 : page[1]);
    return {
        page,
        segment,
        components,
        layoutOrPagePath,
        parallelRoutes
    };
}

//# sourceMappingURL=parse-loader-tree.js.map