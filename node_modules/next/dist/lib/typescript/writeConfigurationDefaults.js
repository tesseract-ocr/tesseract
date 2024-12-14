"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getRequiredConfiguration: null,
    writeConfigurationDefaults: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getRequiredConfiguration: function() {
        return getRequiredConfiguration;
    },
    writeConfigurationDefaults: function() {
        return writeConfigurationDefaults;
    }
});
const _fs = require("fs");
const _picocolors = require("../picocolors");
const _commentjson = /*#__PURE__*/ _interop_require_wildcard(require("next/dist/compiled/comment-json"));
const _semver = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/semver"));
const _os = /*#__PURE__*/ _interop_require_default(require("os"));
const _getTypeScriptConfiguration = require("./getTypeScriptConfiguration");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../../build/output/log"));
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
function getDesiredCompilerOptions(ts, tsOptions) {
    const o = {
        target: {
            suggested: 'ES2017',
            reason: 'For top-level `await`. Note: Next.js only polyfills for the esmodules target.'
        },
        // These are suggested values and will be set when not present in the
        // tsconfig.json
        lib: {
            suggested: [
                'dom',
                'dom.iterable',
                'esnext'
            ]
        },
        allowJs: {
            suggested: true
        },
        skipLibCheck: {
            suggested: true
        },
        strict: {
            suggested: false
        },
        ..._semver.default.lt(ts.version, '5.0.0') ? {
            forceConsistentCasingInFileNames: {
                suggested: true
            }
        } : undefined,
        noEmit: {
            suggested: true
        },
        ..._semver.default.gte(ts.version, '4.4.2') ? {
            incremental: {
                suggested: true
            }
        } : undefined,
        // These values are required and cannot be changed by the user
        // Keep this in sync with the webpack config
        // 'parsedValue' matches the output value from ts.parseJsonConfigFileContent()
        module: {
            parsedValue: ts.ModuleKind.ESNext,
            // All of these values work:
            parsedValues: [
                _semver.default.gte(ts.version, '5.4.0') && ts.ModuleKind.Preserve,
                ts.ModuleKind.ES2020,
                ts.ModuleKind.ESNext,
                ts.ModuleKind.CommonJS,
                ts.ModuleKind.AMD,
                ts.ModuleKind.NodeNext,
                ts.ModuleKind.Node16
            ],
            value: 'esnext',
            reason: 'for dynamic import() support'
        },
        // TODO: Semver check not needed once Next.js repo uses 5.4.
        ..._semver.default.gte(ts.version, '5.4.0') && (tsOptions == null ? void 0 : tsOptions.module) === ts.ModuleKind.Preserve ? {
        } : {
            esModuleInterop: {
                value: true,
                reason: 'requirement for SWC / babel'
            },
            moduleResolution: {
                // In TypeScript 5.0, `NodeJs` has renamed to `Node10`
                parsedValue: ts.ModuleResolutionKind.Bundler ?? ts.ModuleResolutionKind.NodeNext ?? ts.ModuleResolutionKind.Node10 ?? ts.ModuleResolutionKind.NodeJs,
                // All of these values work:
                parsedValues: [
                    ts.ModuleResolutionKind.Node10 ?? ts.ModuleResolutionKind.NodeJs,
                    // only newer TypeScript versions have this field, it
                    // will be filtered for new versions of TypeScript
                    ts.ModuleResolutionKind.Node12,
                    ts.ModuleResolutionKind.Node16,
                    ts.ModuleResolutionKind.NodeNext,
                    ts.ModuleResolutionKind.Bundler
                ].filter((val)=>typeof val !== 'undefined'),
                value: 'node',
                reason: 'to match webpack resolution'
            },
            resolveJsonModule: {
                value: true,
                reason: 'to match webpack resolution'
            }
        },
        ...(tsOptions == null ? void 0 : tsOptions.verbatimModuleSyntax) === true ? undefined : {
            isolatedModules: {
                value: true,
                reason: 'requirement for SWC / Babel'
            }
        },
        jsx: {
            parsedValue: ts.JsxEmit.Preserve,
            value: 'preserve',
            reason: 'next.js implements its own optimized jsx transform'
        }
    };
    return o;
}
function getRequiredConfiguration(ts) {
    const res = {};
    const desiredCompilerOptions = getDesiredCompilerOptions(ts);
    for (const optionKey of Object.keys(desiredCompilerOptions)){
        const ev = desiredCompilerOptions[optionKey];
        if (!('value' in ev)) {
            continue;
        }
        res[optionKey] = ev.parsedValue ?? ev.value;
    }
    return res;
}
async function writeConfigurationDefaults(ts, tsConfigPath, isFirstTimeSetup, hasAppDir, distDir, hasPagesDir) {
    var _userTsConfig_compilerOptions;
    if (isFirstTimeSetup) {
        await _fs.promises.writeFile(tsConfigPath, '{}' + _os.default.EOL);
    }
    const { options: tsOptions, raw: rawConfig } = await (0, _getTypeScriptConfiguration.getTypeScriptConfiguration)(ts, tsConfigPath, true);
    const userTsConfigContent = await _fs.promises.readFile(tsConfigPath, {
        encoding: 'utf8'
    });
    const userTsConfig = _commentjson.parse(userTsConfigContent);
    if (userTsConfig.compilerOptions == null && !('extends' in rawConfig)) {
        userTsConfig.compilerOptions = {};
        isFirstTimeSetup = true;
    }
    const desiredCompilerOptions = getDesiredCompilerOptions(ts, tsOptions);
    const suggestedActions = [];
    const requiredActions = [];
    for (const optionKey of Object.keys(desiredCompilerOptions)){
        const check = desiredCompilerOptions[optionKey];
        if ('suggested' in check) {
            if (!(optionKey in tsOptions)) {
                if (!userTsConfig.compilerOptions) {
                    userTsConfig.compilerOptions = {};
                }
                userTsConfig.compilerOptions[optionKey] = check.suggested;
                suggestedActions.push((0, _picocolors.cyan)(optionKey) + ' was set to ' + (0, _picocolors.bold)(check.suggested) + (check.reason ? ` (${check.reason})` : ''));
            }
        } else if ('value' in check) {
            var _check_parsedValues;
            const ev = tsOptions[optionKey];
            if (!('parsedValues' in check ? (_check_parsedValues = check.parsedValues) == null ? void 0 : _check_parsedValues.includes(ev) : 'parsedValue' in check ? check.parsedValue === ev : check.value === ev)) {
                if (!userTsConfig.compilerOptions) {
                    userTsConfig.compilerOptions = {};
                }
                userTsConfig.compilerOptions[optionKey] = check.value;
                requiredActions.push((0, _picocolors.cyan)(optionKey) + ' was set to ' + (0, _picocolors.bold)(check.value) + ` (${check.reason})`);
            }
        } else {
            // eslint-disable-next-line @typescript-eslint/no-unused-vars
            const _ = check;
        }
    }
    const nextAppTypes = `${distDir}/types/**/*.ts`;
    if (!('include' in rawConfig)) {
        userTsConfig.include = hasAppDir ? [
            'next-env.d.ts',
            nextAppTypes,
            '**/*.ts',
            '**/*.tsx'
        ] : [
            'next-env.d.ts',
            '**/*.ts',
            '**/*.tsx'
        ];
        suggestedActions.push((0, _picocolors.cyan)('include') + ' was set to ' + (0, _picocolors.bold)(hasAppDir ? `['next-env.d.ts', '${nextAppTypes}', '**/*.ts', '**/*.tsx']` : `['next-env.d.ts', '**/*.ts', '**/*.tsx']`));
    } else if (hasAppDir && !rawConfig.include.includes(nextAppTypes)) {
        if (!Array.isArray(userTsConfig.include)) {
            userTsConfig.include = [];
        }
        // rawConfig will resolve all extends and include paths (ex: tsconfig.json, tsconfig.base.json, etc.)
        // if it doesn't match userTsConfig then update the userTsConfig to add the
        // rawConfig's includes in addition to nextAppTypes
        if (rawConfig.include.length !== userTsConfig.include.length || JSON.stringify(rawConfig.include.sort()) !== JSON.stringify(userTsConfig.include.sort())) {
            userTsConfig.include.push(...rawConfig.include, nextAppTypes);
            suggestedActions.push((0, _picocolors.cyan)('include') + ' was set to ' + (0, _picocolors.bold)(`[${[
                ...rawConfig.include,
                nextAppTypes
            ].map((i)=>`'${i}'`).join(', ')}]`));
        } else {
            userTsConfig.include.push(nextAppTypes);
            suggestedActions.push((0, _picocolors.cyan)('include') + ' was updated to add ' + (0, _picocolors.bold)(`'${nextAppTypes}'`));
        }
    }
    // Enable the Next.js typescript plugin.
    if (hasAppDir) {
        // Check if the config or the resolved config has the plugin already.
        const plugins = [
            ...Array.isArray(tsOptions.plugins) ? tsOptions.plugins : [],
            ...userTsConfig.compilerOptions && Array.isArray(userTsConfig.compilerOptions.plugins) ? userTsConfig.compilerOptions.plugins : []
        ];
        const hasNextPlugin = plugins.some(({ name })=>name === 'next');
        // If the TS config extends on another config, we can't add the `plugin` field
        // because that will override the parent config's plugins.
        // Instead we have to show a message to the user to add the plugin manually.
        if (!userTsConfig.compilerOptions || plugins.length && !hasNextPlugin && 'extends' in rawConfig && (!rawConfig.compilerOptions || !rawConfig.compilerOptions.plugins)) {
            _log.info(`\nYour ${(0, _picocolors.bold)('tsconfig.json')} extends another configuration, which means we cannot add the Next.js TypeScript plugin automatically. To improve your development experience, we recommend adding the Next.js plugin (\`${(0, _picocolors.cyan)('"plugins": [{ "name": "next" }]')}\`) manually to your TypeScript configuration. Learn more: https://nextjs.org/docs/app/api-reference/config/typescript#the-typescript-plugin\n`);
        } else if (!hasNextPlugin) {
            if (!('plugins' in userTsConfig.compilerOptions)) {
                userTsConfig.compilerOptions.plugins = [];
            }
            userTsConfig.compilerOptions.plugins.push({
                name: 'next'
            });
            suggestedActions.push((0, _picocolors.cyan)('plugins') + ' was updated to add ' + (0, _picocolors.bold)(`{ name: 'next' }`));
        }
        // If `strict` is set to `false` and `strictNullChecks` is set to `false`,
        // then set `strictNullChecks` to `true`.
        if (hasPagesDir && hasAppDir && !tsOptions.strict && !('strictNullChecks' in tsOptions)) {
            userTsConfig.compilerOptions.strictNullChecks = true;
            suggestedActions.push((0, _picocolors.cyan)('strictNullChecks') + ' was set to ' + (0, _picocolors.bold)(`true`));
        }
    }
    if (!('exclude' in rawConfig)) {
        userTsConfig.exclude = [
            'node_modules'
        ];
        suggestedActions.push((0, _picocolors.cyan)('exclude') + ' was set to ' + (0, _picocolors.bold)(`['node_modules']`));
    }
    if (suggestedActions.length < 1 && requiredActions.length < 1) {
        return;
    }
    await _fs.promises.writeFile(tsConfigPath, _commentjson.stringify(userTsConfig, null, 2) + _os.default.EOL);
    _log.info('');
    if (isFirstTimeSetup) {
        _log.info(`We detected TypeScript in your project and created a ${(0, _picocolors.cyan)('tsconfig.json')} file for you.`);
        return;
    }
    _log.info(`We detected TypeScript in your project and reconfigured your ${(0, _picocolors.cyan)('tsconfig.json')} file for you.${((_userTsConfig_compilerOptions = userTsConfig.compilerOptions) == null ? void 0 : _userTsConfig_compilerOptions.strict) ? '' : ` Strict-mode is set to ${(0, _picocolors.cyan)('false')} by default.`}`);
    if (suggestedActions.length) {
        _log.info(`The following suggested values were added to your ${(0, _picocolors.cyan)('tsconfig.json')}. These values ${(0, _picocolors.cyan)('can be changed')} to fit your project's needs:\n`);
        suggestedActions.forEach((action)=>_log.info(`\t- ${action}`));
        _log.info('');
    }
    if (requiredActions.length) {
        _log.info(`The following ${(0, _picocolors.white)('mandatory changes')} were made to your ${(0, _picocolors.cyan)('tsconfig.json')}:\n`);
        requiredActions.forEach((action)=>_log.info(`\t- ${action}`));
        _log.info('');
    }
}

//# sourceMappingURL=writeConfigurationDefaults.js.map