"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getNpxCommand", {
    enumerable: true,
    get: function() {
        return getNpxCommand;
    }
});
const _child_process = require("child_process");
const _getpkgmanager = require("./get-pkg-manager");
function getNpxCommand(baseDir) {
    const pkgManager = (0, _getpkgmanager.getPkgManager)(baseDir);
    let command = "npx";
    if (pkgManager === "pnpm") {
        command = "pnpm dlx";
    } else if (pkgManager === "yarn") {
        try {
            (0, _child_process.execSync)("yarn dlx --help", {
                stdio: "ignore"
            });
            command = "yarn dlx";
        } catch  {}
    }
    return command;
}

//# sourceMappingURL=get-npx-command.js.map