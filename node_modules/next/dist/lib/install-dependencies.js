"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "installDependencies", {
    enumerable: true,
    get: function() {
        return installDependencies;
    }
});
const _picocolors = require("./picocolors");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _getpkgmanager = require("./helpers/get-pkg-manager");
const _install = require("./helpers/install");
const _getonline = require("./helpers/get-online");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function installDependencies(baseDir, deps, dev = false) {
    const packageManager = (0, _getpkgmanager.getPkgManager)(baseDir);
    const isOnline = await (0, _getonline.getOnline)();
    if (deps.length) {
        console.log();
        console.log(`Installing ${dev ? "devDependencies" : "dependencies"} (${packageManager}):`);
        for (const dep of deps){
            console.log(`- ${(0, _picocolors.cyan)(dep.pkg)}`);
        }
        console.log();
        await (0, _install.install)(_path.default.resolve(baseDir), deps.map((dep)=>dep.pkg), {
            devDependencies: dev,
            isOnline,
            packageManager
        });
        console.log();
    }
}

//# sourceMappingURL=install-dependencies.js.map