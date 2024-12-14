"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getPagePaths", {
    enumerable: true,
    get: function() {
        return getPagePaths;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _denormalizepagepath = require("./denormalize-page-path");
const _path = /*#__PURE__*/ _interop_require_default._(require("../isomorphic/path"));
function getPagePaths(normalizedPagePath, extensions, isAppDir) {
    const page = (0, _denormalizepagepath.denormalizePagePath)(normalizedPagePath);
    let prefixes;
    if (isAppDir) {
        prefixes = [
            page
        ];
    } else if (normalizedPagePath.endsWith('/index')) {
        prefixes = [
            _path.default.join(page, 'index')
        ];
    } else {
        prefixes = [
            page,
            _path.default.join(page, 'index')
        ];
    }
    const paths = [];
    for (const extension of extensions){
        for (const prefix of prefixes){
            paths.push(prefix + "." + extension);
        }
    }
    return paths;
}

//# sourceMappingURL=get-page-paths.js.map