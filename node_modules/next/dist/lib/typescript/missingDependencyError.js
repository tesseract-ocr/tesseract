"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "missingDepsError", {
    enumerable: true,
    get: function() {
        return missingDepsError;
    }
});
const _picocolors = require("../picocolors");
const _oxfordcommalist = require("../oxford-comma-list");
const _fatalerror = require("../fatal-error");
const _getpkgmanager = require("../helpers/get-pkg-manager");
function missingDepsError(dir, missingPackages) {
    const packagesHuman = (0, _oxfordcommalist.getOxfordCommaList)(missingPackages.map((p)=>p.pkg));
    const packagesCli = missingPackages.map((p)=>p.pkg).join(" ");
    const packageManager = (0, _getpkgmanager.getPkgManager)(dir);
    const removalMsg = "\n\n" + (0, _picocolors.bold)("If you are not trying to use TypeScript, please remove the " + (0, _picocolors.cyan)("tsconfig.json") + " file from your package root (and any TypeScript files in your pages directory).");
    throw new _fatalerror.FatalError((0, _picocolors.bold)((0, _picocolors.red)(`It looks like you're trying to use TypeScript but do not have the required package(s) installed.`)) + "\n\n" + (0, _picocolors.bold)(`Please install ${(0, _picocolors.bold)(packagesHuman)} by running:`) + "\n\n" + `\t${(0, _picocolors.bold)((0, _picocolors.cyan)((packageManager === "yarn" ? "yarn add --dev" : packageManager === "pnpm" ? "pnpm install --save-dev" : "npm install --save-dev") + " " + packagesCli))}` + removalMsg + "\n");
}

//# sourceMappingURL=missingDependencyError.js.map