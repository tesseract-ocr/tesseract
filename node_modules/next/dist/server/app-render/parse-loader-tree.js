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
    const [segment, parallelRoutes, modules] = tree;
    const { layout } = modules;
    let { page } = modules;
    // a __DEFAULT__ segment means that this route didn't match any of the
    // segments in the route, so we should use the default page
    page = segment === _segment.DEFAULT_SEGMENT_KEY ? modules.defaultPage : page;
    const layoutOrPagePath = (layout == null ? void 0 : layout[1]) || (page == null ? void 0 : page[1]);
    return {
        page,
        segment,
        modules,
        layoutOrPagePath,
        parallelRoutes
    };
}

//# sourceMappingURL=parse-loader-tree.js.map