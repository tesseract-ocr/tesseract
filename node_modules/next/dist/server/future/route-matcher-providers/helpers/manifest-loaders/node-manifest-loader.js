"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "NodeManifestLoader", {
    enumerable: true,
    get: function() {
        return NodeManifestLoader;
    }
});
const _constants = require("../../../../../shared/lib/constants");
const _path = /*#__PURE__*/ _interop_require_default(require("../../../../../shared/lib/isomorphic/path"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
class NodeManifestLoader {
    constructor(distDir){
        this.distDir = distDir;
    }
    static require(id) {
        try {
            return require(id);
        } catch  {
            return null;
        }
    }
    load(name) {
        return NodeManifestLoader.require(_path.default.join(this.distDir, _constants.SERVER_DIRECTORY, name));
    }
}

//# sourceMappingURL=node-manifest-loader.js.map