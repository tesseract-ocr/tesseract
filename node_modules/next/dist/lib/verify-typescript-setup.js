"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "verifyTypeScriptSetup", {
    enumerable: true,
    get: function() {
        return verifyTypeScriptSetup;
    }
});
const _picocolors = require("./picocolors");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _hasnecessarydependencies = require("./has-necessary-dependencies");
const _semver = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/semver"));
const _compileerror = require("./compile-error");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../build/output/log"));
const _getTypeScriptIntent = require("./typescript/getTypeScriptIntent");
const _writeAppTypeDeclarations = require("./typescript/writeAppTypeDeclarations");
const _writeConfigurationDefaults = require("./typescript/writeConfigurationDefaults");
const _installdependencies = require("./install-dependencies");
const _ciinfo = require("../telemetry/ci-info");
const _missingDependencyError = require("./typescript/missingDependencyError");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function _getRequireWildcardCache(nodeInterop) {
    if (typeof WeakMap !== "function") return null;
    var cacheBabelInterop = new WeakMap();
    var cacheNodeInterop = new WeakMap();
    return (_getRequireWildcardCache = function(nodeInterop) {
        return nodeInterop ? cacheNodeInterop : cacheBabelInterop;
    })(nodeInterop);
}
function _interop_require_wildcard(obj, nodeInterop) {
    if (!nodeInterop && obj && obj.__esModule) {
        return obj;
    }
    if (obj === null || typeof obj !== "object" && typeof obj !== "function") {
        return {
            default: obj
        };
    }
    var cache = _getRequireWildcardCache(nodeInterop);
    if (cache && cache.has(obj)) {
        return cache.get(obj);
    }
    var newObj = {
        __proto__: null
    };
    var hasPropertyDescriptor = Object.defineProperty && Object.getOwnPropertyDescriptor;
    for(var key in obj){
        if (key !== "default" && Object.prototype.hasOwnProperty.call(obj, key)) {
            var desc = hasPropertyDescriptor ? Object.getOwnPropertyDescriptor(obj, key) : null;
            if (desc && (desc.get || desc.set)) {
                Object.defineProperty(newObj, key, desc);
            } else {
                newObj[key] = obj[key];
            }
        }
    }
    newObj.default = obj;
    if (cache) {
        cache.set(obj, newObj);
    }
    return newObj;
}
const requiredPackages = [
    {
        file: "typescript/lib/typescript.js",
        pkg: "typescript",
        exportsRestrict: true
    },
    {
        file: "@types/react/index.d.ts",
        pkg: "@types/react",
        exportsRestrict: true
    },
    {
        file: "@types/node/index.d.ts",
        pkg: "@types/node",
        exportsRestrict: true
    }
];
async function verifyTypeScriptSetup({ dir, distDir, cacheDir, intentDirs, tsconfigPath, typeCheckPreflight, disableStaticImages, hasAppDir, hasPagesDir }) {
    const resolvedTsConfigPath = _path.default.join(dir, tsconfigPath);
    try {
        var _deps_missing;
        // Check if the project uses TypeScript:
        const intent = await (0, _getTypeScriptIntent.getTypeScriptIntent)(dir, intentDirs, tsconfigPath);
        if (!intent) {
            return {
                version: null
            };
        }
        // Ensure TypeScript and necessary `@types/*` are installed:
        let deps = await (0, _hasnecessarydependencies.hasNecessaryDependencies)(dir, requiredPackages);
        if (((_deps_missing = deps.missing) == null ? void 0 : _deps_missing.length) > 0) {
            if (_ciinfo.isCI) {
                // we don't attempt auto install in CI to avoid side-effects
                // and instead log the error for installing needed packages
                (0, _missingDependencyError.missingDepsError)(dir, deps.missing);
            }
            console.log((0, _picocolors.bold)((0, _picocolors.yellow)(`It looks like you're trying to use TypeScript but do not have the required package(s) installed.`)) + "\n" + "Installing dependencies" + "\n\n" + (0, _picocolors.bold)("If you are not trying to use TypeScript, please remove the " + (0, _picocolors.cyan)("tsconfig.json") + " file from your package root (and any TypeScript files in your pages directory).") + "\n");
            await (0, _installdependencies.installDependencies)(dir, deps.missing, true).catch((err)=>{
                if (err && typeof err === "object" && "command" in err) {
                    console.error(`Failed to install required TypeScript dependencies, please install them manually to continue:\n` + err.command + "\n");
                }
                throw err;
            });
            deps = await (0, _hasnecessarydependencies.hasNecessaryDependencies)(dir, requiredPackages);
        }
        // Load TypeScript after we're sure it exists:
        const tsPath = deps.resolved.get("typescript");
        const ts = await Promise.resolve(require(tsPath));
        if (_semver.default.lt(ts.version, "4.5.2")) {
            _log.warn(`Minimum recommended TypeScript version is v4.5.2, older versions can potentially be incompatible with Next.js. Detected: ${ts.version}`);
        }
        // Reconfigure (or create) the user's `tsconfig.json` for them:
        await (0, _writeConfigurationDefaults.writeConfigurationDefaults)(ts, resolvedTsConfigPath, intent.firstTimeSetup, hasAppDir, distDir, hasPagesDir);
        // Write out the necessary `next-env.d.ts` file to correctly register
        // Next.js' types:
        await (0, _writeAppTypeDeclarations.writeAppTypeDeclarations)({
            baseDir: dir,
            imageImportsEnabled: !disableStaticImages,
            hasPagesDir,
            hasAppDir
        });
        let result;
        if (typeCheckPreflight) {
            const { runTypeCheck } = require("./typescript/runTypeCheck");
            // Verify the project passes type-checking before we go to webpack phase:
            result = await runTypeCheck(ts, dir, distDir, resolvedTsConfigPath, cacheDir, hasAppDir);
        }
        return {
            result,
            version: ts.version
        };
    } catch (err) {
        // These are special errors that should not show a stack trace:
        if (err instanceof _compileerror.CompileError) {
            console.error((0, _picocolors.red)("Failed to compile.\n"));
            console.error(err.message);
            process.exit(1);
        }
        /**
     * verifyTypeScriptSetup can be either invoked directly in the main thread (during next dev / next lint)
     * or run in a worker (during next build). In the latter case, we need to print the error message, as the
     * parent process will only receive an `Jest worker encountered 1 child process exceptions, exceeding retry limit`.
     */ // we are in a worker, print the error message and exit the process
        if (process.env.JEST_WORKER_ID) {
            if (err instanceof Error) {
                console.error(err.message);
            } else {
                console.error(err);
            }
            process.exit(1);
        }
        // we are in the main thread, throw the error and it will be handled by the caller
        throw err;
    }
}

//# sourceMappingURL=verify-typescript-setup.js.map