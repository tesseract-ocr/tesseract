import { cyan } from "./picocolors";
import path from "path";
import { getPkgManager } from "./helpers/get-pkg-manager";
import { install } from "./helpers/install";
import { getOnline } from "./helpers/get-online";
export async function installDependencies(baseDir, deps, dev = false) {
    const packageManager = getPkgManager(baseDir);
    const isOnline = await getOnline();
    if (deps.length) {
        console.log();
        console.log(`Installing ${dev ? "devDependencies" : "dependencies"} (${packageManager}):`);
        for (const dep of deps){
            console.log(`- ${cyan(dep.pkg)}`);
        }
        console.log();
        await install(path.resolve(baseDir), deps.map((dep)=>dep.pkg), {
            devDependencies: dev,
            isOnline,
            packageManager
        });
        console.log();
    }
}

//# sourceMappingURL=install-dependencies.js.map