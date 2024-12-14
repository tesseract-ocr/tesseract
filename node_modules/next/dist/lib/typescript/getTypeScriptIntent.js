"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getTypeScriptIntent", {
    enumerable: true,
    get: function() {
        return getTypeScriptIntent;
    }
});
const _fs = require("fs");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _recursivereaddir = require("../recursive-readdir");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function getTypeScriptIntent(baseDir, intentDirs, tsconfigPath) {
    const resolvedTsConfigPath = _path.default.join(baseDir, tsconfigPath);
    // The integration turns on if we find a `tsconfig.json` in the user's
    // project.
    const hasTypeScriptConfiguration = (0, _fs.existsSync)(resolvedTsConfigPath);
    if (hasTypeScriptConfiguration) {
        const content = await _fs.promises.readFile(resolvedTsConfigPath, {
            encoding: 'utf8'
        }).then((txt)=>txt.trim(), ()=>null);
        return {
            firstTimeSetup: content === '' || content === '{}'
        };
    }
    // Next.js also offers a friendly setup mode that bootstraps a TypeScript
    // project for the user when we detect TypeScript files. So, we need to check
    // the `pages/` directory for a TypeScript file.
    // Checking all directories is too slow, so this is a happy medium.
    const tsFilesRegex = /.*\.(ts|tsx)$/;
    const excludedRegex = /(node_modules|.*\.d\.ts$)/;
    for (const dir of intentDirs){
        const typescriptFiles = await (0, _recursivereaddir.recursiveReadDir)(dir, {
            pathnameFilter: (name)=>tsFilesRegex.test(name),
            ignoreFilter: (name)=>excludedRegex.test(name)
        });
        if (typescriptFiles.length) {
            return {
                firstTimeSetup: true
            };
        }
    }
    return false;
}

//# sourceMappingURL=getTypeScriptIntent.js.map