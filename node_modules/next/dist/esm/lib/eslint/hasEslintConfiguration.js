import { promises as fs } from "fs";
export async function hasEslintConfiguration(eslintrcFile, packageJsonConfig) {
    const configObject = {
        exists: false,
        emptyEslintrc: false,
        emptyPkgJsonConfig: false
    };
    if (eslintrcFile) {
        const content = await fs.readFile(eslintrcFile, {
            encoding: "utf8"
        }).then((txt)=>txt.trim().replace(/\n/g, ""), ()=>null);
        if (content === "" || content === "{}" || content === "---" || content === "module.exports = {}") {
            configObject.emptyEslintrc = true;
        } else {
            configObject.exists = true;
        }
    } else if (packageJsonConfig == null ? void 0 : packageJsonConfig.eslintConfig) {
        if (Object.keys(packageJsonConfig.eslintConfig).length) {
            configObject.exists = true;
        } else {
            configObject.emptyPkgJsonConfig = true;
        }
    }
    return configObject;
}

//# sourceMappingURL=hasEslintConfiguration.js.map