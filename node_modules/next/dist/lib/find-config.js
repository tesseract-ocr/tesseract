"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    findConfig: null,
    findConfigPath: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    findConfig: function() {
        return findConfig;
    },
    findConfigPath: function() {
        return findConfigPath;
    }
});
const _findup = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/find-up"));
const _promises = require("fs/promises");
const _json5 = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/json5"));
const _url = require("url");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function findConfigPath(dir, key) {
    // If we didn't find the configuration in `package.json`, we should look for
    // known filenames.
    return (0, _findup.default)([
        `.${key}rc.json`,
        `${key}.config.json`,
        `.${key}rc.js`,
        `${key}.config.js`,
        `${key}.config.mjs`,
        `${key}.config.cjs`
    ], {
        cwd: dir
    });
}
async function findConfig(directory, key, _returnFile) {
    // `package.json` configuration always wins. Let's check that first.
    const packageJsonPath = await (0, _findup.default)("package.json", {
        cwd: directory
    });
    let isESM = false;
    if (packageJsonPath) {
        try {
            const packageJsonStr = await (0, _promises.readFile)(packageJsonPath, "utf8");
            const packageJson = JSON.parse(packageJsonStr);
            if (typeof packageJson !== "object") {
                throw new Error() // Stop processing and continue
                ;
            }
            if (packageJson.type === "module") {
                isESM = true;
            }
            if (packageJson[key] != null && typeof packageJson[key] === "object") {
                return packageJson[key];
            }
        } catch  {
        // Ignore error and continue
        }
    }
    const filePath = await findConfigPath(directory, key);
    const esmImport = (path)=>{
        // Skip mapping to absolute url with pathToFileURL on windows if it's jest
        // https://github.com/nodejs/node/issues/31710#issuecomment-587345749
        if (process.platform === "win32" && !process.env.JEST_WORKER_ID) {
            // on windows import("C:\\path\\to\\file") is not valid, so we need to
            // use file:// URLs
            return import((0, _url.pathToFileURL)(path).toString());
        } else {
            return import(path);
        }
    };
    if (filePath) {
        if (filePath.endsWith(".js")) {
            if (isESM) {
                return (await esmImport(filePath)).default;
            } else {
                return require(filePath);
            }
        } else if (filePath.endsWith(".mjs")) {
            return (await esmImport(filePath)).default;
        } else if (filePath.endsWith(".cjs")) {
            return require(filePath);
        }
        // We load JSON contents with JSON5 to allow users to comment in their
        // configuration file. This pattern was popularized by TypeScript.
        const fileContents = await (0, _promises.readFile)(filePath, "utf8");
        return _json5.default.parse(fileContents);
    }
    return null;
}

//# sourceMappingURL=find-config.js.map