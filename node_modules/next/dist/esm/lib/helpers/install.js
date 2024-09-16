import { yellow } from "../picocolors";
import spawn from "next/dist/compiled/cross-spawn";
/**
 * Spawn a package manager installation with either Yarn or NPM.
 *
 * @returns A Promise that resolves once the installation is finished.
 */ export function install(root, dependencies, { packageManager, isOnline, devDependencies }) {
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
                console.log(yellow("You appear to be offline."));
                if (useYarn) {
                    console.log(yellow("Falling back to the local Yarn cache."));
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
     */ const child = spawn(command, args, {
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