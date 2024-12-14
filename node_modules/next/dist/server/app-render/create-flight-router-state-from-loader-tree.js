"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "createFlightRouterStateFromLoaderTree", {
    enumerable: true,
    get: function() {
        return createFlightRouterStateFromLoaderTree;
    }
});
const _segment = require("../../shared/lib/segment");
function createFlightRouterStateFromLoaderTree([segment, parallelRoutes, { layout }], getDynamicParamFromSegment, searchParams, rootLayoutIncluded = false) {
    const dynamicParam = getDynamicParamFromSegment(segment);
    const treeSegment = dynamicParam ? dynamicParam.treeSegment : segment;
    const segmentTree = [
        (0, _segment.addSearchParamsIfPageSegment)(treeSegment, searchParams),
        {}
    ];
    if (!rootLayoutIncluded && typeof layout !== 'undefined') {
        rootLayoutIncluded = true;
        segmentTree[4] = true;
    }
    segmentTree[1] = Object.keys(parallelRoutes).reduce((existingValue, currentValue)=>{
        existingValue[currentValue] = createFlightRouterStateFromLoaderTree(parallelRoutes[currentValue], getDynamicParamFromSegment, searchParams, rootLayoutIncluded);
        return existingValue;
    }, {});
    return segmentTree;
}

//# sourceMappingURL=create-flight-router-state-from-loader-tree.js.map