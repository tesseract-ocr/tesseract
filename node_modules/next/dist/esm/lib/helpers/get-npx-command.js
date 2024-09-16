import { execSync } from "child_process";
import { getPkgManager } from "./get-pkg-manager";
export function getNpxCommand(baseDir) {
    const pkgManager = getPkgManager(baseDir);
    let command = "npx";
    if (pkgManager === "pnpm") {
        command = "pnpm dlx";
    } else if (pkgManager === "yarn") {
        try {
            execSync("yarn dlx --help", {
                stdio: "ignore"
            });
            command = "yarn dlx";
        } catch  {}
    }
    return command;
}

//# sourceMappingURL=get-npx-command.js.map