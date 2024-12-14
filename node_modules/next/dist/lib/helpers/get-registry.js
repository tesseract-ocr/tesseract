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
    const pkgManager = (0, _getpkgmanager.getPkgManager)(baseDir);
    // Since `npm config` command fails in npm workspace to prevent workspace config conflicts,
    // add `--no-workspaces` flag to run under the context of the root project only.
    // Safe for non-workspace projects as it's equivalent to default `--workspaces=false`.
    // x-ref: https://github.com/vercel/next.js/issues/47121#issuecomment-1499044345
    // x-ref: https://github.com/npm/statusboard/issues/371#issue-920669998
    const resolvedFlags = pkgManager === 'npm' ? '--no-workspaces' : '';
    let registry = `https://registry.npmjs.org/`;
    try {
        const output = (0, _child_process.execSync)(`${pkgManager} config get registry ${resolvedFlags}`, {
            env: {
                ...process.env,
                NODE_OPTIONS: (0, _utils.getFormattedNodeOptionsWithoutInspect)()
            }
        }).toString().trim();
        if (output.startsWith('http')) {
            registry = output.endsWith('/') ? output : `${output}/`;
        }
    } catch (err) {
        throw new Error(`Failed to get registry from "${pkgManager}".`, {
            cause: err
        });
    }
    return registry;
}

//# sourceMappingURL=get-registry.js.map