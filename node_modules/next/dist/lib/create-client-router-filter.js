"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "createClientRouterFilter", {
    enumerable: true,
    get: function() {
        return createClientRouterFilter;
    }
});
const _bloomfilter = require("../shared/lib/bloom-filter");
const _utils = require("../shared/lib/router/utils");
const _removetrailingslash = require("../shared/lib/router/utils/remove-trailing-slash");
const _trytoparsepath = require("./try-to-parse-path");
const _interceptionroutes = require("../server/lib/interception-routes");
function createClientRouterFilter(paths, redirects, allowedErrorRate) {
    const staticPaths = new Set();
    const dynamicPaths = new Set();
    for (let path of paths){
        if ((0, _utils.isDynamicRoute)(path)) {
            if ((0, _interceptionroutes.isInterceptionRouteAppPath)(path)) {
                path = (0, _interceptionroutes.extractInterceptionRouteInformation)(path).interceptedRoute;
            }
            let subPath = '';
            const pathParts = path.split('/');
            // start at 1 since we split on '/' and the path starts
            // with this so the first entry is an empty string
            for(let i = 1; i < pathParts.length + 1; i++){
                const curPart = pathParts[i];
                if (curPart.startsWith('[')) {
                    break;
                }
                subPath = `${subPath}/${curPart}`;
            }
            if (subPath) {
                dynamicPaths.add(subPath);
            }
        } else {
            staticPaths.add(path);
        }
    }
    for (const redirect of redirects){
        const { source } = redirect;
        const path = (0, _removetrailingslash.removeTrailingSlash)(source);
        let tokens = [];
        try {
            tokens = (0, _trytoparsepath.tryToParsePath)(source).tokens || [];
        } catch  {}
        if (tokens.every((token)=>typeof token === 'string')) {
            // only include static redirects initially
            staticPaths.add(path);
        }
    }
    const staticFilter = _bloomfilter.BloomFilter.from([
        ...staticPaths
    ], allowedErrorRate);
    const dynamicFilter = _bloomfilter.BloomFilter.from([
        ...dynamicPaths
    ], allowedErrorRate);
    const data = {
        staticFilter: staticFilter.export(),
        dynamicFilter: dynamicFilter.export()
    };
    return data;
}

//# sourceMappingURL=create-client-router-filter.js.map