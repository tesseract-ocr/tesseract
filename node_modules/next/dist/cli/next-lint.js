#!/usr/bin/env node
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "nextLint", {
    enumerable: true,
    get: function() {
        return nextLint;
    }
});
const _fs = require("fs");
const _path = require("path");
const _config = /*#__PURE__*/ _interop_require_default(require("../server/config"));
const _utils = require("../server/lib/utils");
const _storage = require("../telemetry/storage");
const _picocolors = require("../lib/picocolors");
const _constants = require("../lib/constants");
const _runLintCheck = require("../lib/eslint/runLintCheck");
const _compileerror = require("../lib/compile-error");
const _constants1 = require("../shared/lib/constants");
const _events = require("../telemetry/events");
const _getprojectdir = require("../lib/get-project-dir");
const _findpagesdir = require("../lib/find-pages-dir");
const _verifytypescriptsetup = require("../lib/verify-typescript-setup");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const eslintOptions = (options, defaultCacheLocation)=>({
        overrideConfigFile: options.config || null,
        extensions: options.ext ?? [],
        resolvePluginsRelativeTo: options.resolvePluginsRelativeTo || null,
        rulePaths: options.rulesdir ?? [],
        fix: options.fix ?? false,
        fixTypes: options.fixType ?? null,
        ignorePath: options.ignorePath || null,
        ignore: options.ignore,
        allowInlineConfig: options.inlineConfig,
        reportUnusedDisableDirectives: options.reportUnusedDisableDirectivesSeverity || null,
        cache: options.cache,
        cacheLocation: options.cacheLocation || defaultCacheLocation,
        cacheStrategy: options.cacheStrategy,
        errorOnUnmatchedPattern: options.errorOnUnmatchedPattern ?? false
    });
const nextLint = async (options, directory)=>{
    var _nextConfig_eslint;
    const baseDir = (0, _getprojectdir.getProjectDir)(directory);
    // Check if the provided directory exists
    if (!(0, _fs.existsSync)(baseDir)) {
        (0, _utils.printAndExit)(`> No such directory exists as the project root: ${baseDir}`);
    }
    const nextConfig = await (0, _config.default)(_constants1.PHASE_PRODUCTION_BUILD, baseDir);
    const files = options.file ?? [];
    const dirs = options.dir ?? ((_nextConfig_eslint = nextConfig.eslint) == null ? void 0 : _nextConfig_eslint.dirs);
    const filesToLint = [
        ...dirs ?? [],
        ...files
    ];
    const pathsToLint = (filesToLint.length ? filesToLint : _constants.ESLINT_DEFAULT_DIRS).reduce((res, d)=>{
        const currDir = (0, _path.join)(baseDir, d);
        if (!(0, _fs.existsSync)(currDir)) {
            return res;
        }
        res.push(currDir);
        return res;
    }, []);
    const reportErrorsOnly = Boolean(options.quiet);
    const maxWarnings = options.maxWarnings;
    const formatter = options.format || null;
    const strict = Boolean(options.strict);
    const outputFile = options.outputFile || null;
    const distDir = (0, _path.join)(baseDir, nextConfig.distDir);
    const defaultCacheLocation = (0, _path.join)(distDir, "cache", "eslint/");
    const { pagesDir, appDir } = (0, _findpagesdir.findPagesDir)(baseDir);
    await (0, _verifytypescriptsetup.verifyTypeScriptSetup)({
        dir: baseDir,
        distDir: nextConfig.distDir,
        intentDirs: [
            pagesDir,
            appDir
        ].filter(Boolean),
        typeCheckPreflight: false,
        tsconfigPath: nextConfig.typescript.tsconfigPath,
        disableStaticImages: nextConfig.images.disableStaticImages,
        hasAppDir: !!appDir,
        hasPagesDir: !!pagesDir
    });
    (0, _runLintCheck.runLintCheck)(baseDir, pathsToLint, {
        lintDuringBuild: false,
        eslintOptions: eslintOptions(options, defaultCacheLocation),
        reportErrorsOnly,
        maxWarnings,
        formatter,
        outputFile,
        strict
    }).then(async (lintResults)=>{
        const lintOutput = typeof lintResults === "string" ? lintResults : lintResults == null ? void 0 : lintResults.output;
        if (typeof lintResults !== "string" && (lintResults == null ? void 0 : lintResults.eventInfo)) {
            const telemetry = new _storage.Telemetry({
                distDir
            });
            telemetry.record((0, _events.eventLintCheckCompleted)({
                ...lintResults.eventInfo,
                buildLint: false
            }));
            await telemetry.flush();
        }
        if (typeof lintResults !== "string" && (lintResults == null ? void 0 : lintResults.isError) && lintOutput) {
            throw new _compileerror.CompileError(lintOutput);
        }
        if (lintOutput) {
            (0, _utils.printAndExit)(lintOutput, 0);
        } else if (lintResults && !lintOutput) {
            (0, _utils.printAndExit)((0, _picocolors.green)("✔ No ESLint warnings or errors"), 0);
        } else {
            // this makes sure we exit 1 after the error from line 116
            // in packages/next/src/lib/eslint/runLintCheck
            process.exit(1);
        }
    }).catch((err)=>{
        (0, _utils.printAndExit)(err.message);
    });
};

//# sourceMappingURL=next-lint.js.map