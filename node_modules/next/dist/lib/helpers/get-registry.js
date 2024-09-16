"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getRegistry", {
    enumerable: true,
    get: function() {
        return getRegistry;
    }
});
const _child_process = require("child_process");
const _getpkgmanager = require("./get-pkg-manager");
const _utils = require("../../server/lib/utils");
function getRegistry(baseDir = process.cwd()) {
    let registry = `https://registry.npmjs.org/`;
    try {
        const pkgManager = (0, _getpkgmanager.getPkgManager)(baseDir);
        const output = (0, _child_process.execSync)(`${pkgManager} config get registry`, {
            env: {
                ...process.env,
                NODE_OPTIONS: (0, _utils.getNodeOptionsWithoutInspect)()
            }
        }).toString().trim();
        if (output.startsWith("http")) {
            registry = output.endsWith("/") ? output : `${output}/`;
        }
    } finally{
        return registry;
    }
}

//# sourceMappingURL=get-registry.js.map