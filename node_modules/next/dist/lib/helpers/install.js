"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "install", {
    enumerable: true,
    get: function() {
        return install;
    }
});
const _picocolors = require("../picocolors");
const _crossspawn = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/cross-spawn"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function install(root, dependencies, { packageManager, isOnline, devDependencies }) {
    /**
   * (p)npm-specific command-line flags.
   */ const npmFlags = [];
    /**
   * Yarn-specific command-line flags.
   */ const yarnFlags = [];
    /**
   * Return a Promise that resolves once the installation is finished.
   */ return new Promise((resolve, reject)=>{
        let args;
        let command = packageManager;
        const useYarn = packageManager === "yarn";
        if (dependencies && dependencies.length) {
            /**
       * If there are dependencies, run a variation of `{packageManager} add`.
       */ if (useYarn) {
                /**
         * Call `yarn add --exact (--offline)? (-D)? ...`.
         */ args = [
                    "add",
                    "--exact"
                ];
                if (!isOnline) args.push("--offline");
                args.push("--cwd", root);
                if (devDependencies) args.push("--dev");
                args.push(...dependencies);
            } else {
                /**
         * Call `(p)npm install [--save|--save-dev] ...`.
         */ args = [
                    "install",
                    "--save-exact"
                ];
                args.push(devDependencies ? "--save-dev" : "--save");
                args.push(...dependencies);
            }
        } else {
            /**
       * If there are no dependencies, run a variation of `{packageManager}
       * install`.
       */ args = [
                "install"
            ];
            if (!isOnline) {
                console.log((0, _picocolors.yellow)("You appear to be offline."));
                if (useYarn) {
                    console.log((0, _picocolors.yellow)("Falling back to the local Yarn cache."));
                    console.log();
                    args.push("--offline");
                } else {
                    console.log();
                }
            }
        }
        /**
     * Add any package manager-specific flags.
     */ if (useYarn) {
            args.push(...yarnFlags);
        } else {
            args.push(...npmFlags);
        }
        /**
     * Spawn the installation process.
     */ const child = (0, _crossspawn.default)(command, args, {
            stdio: "inherit",
            env: {
                ...process.env,
                ADBLOCK: "1",
                // we set NODE_ENV to development as pnpm skips dev
                // dependencies when production
                NODE_ENV: "development",
                DISABLE_OPENCOLLECTIVE: "1"
            }
        });
        child.on("close", (code)=>{
            if (code !== 0) {
                reject({
                    command: `${command} ${args.join(" ")}`
                });
                return;
            }
            resolve();
        });
    });
}

//# sourceMappingURL=install.js.map