import { SERVER_DIRECTORY } from "../../../../../shared/lib/constants";
import path from "../../../../../shared/lib/isomorphic/path";
export class NodeManifestLoader {
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
        return NodeManifestLoader.require(path.join(this.distDir, SERVER_DIRECTORY, name));
    }
}

//# sourceMappingURL=node-manifest-loader.js.map