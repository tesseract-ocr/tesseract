"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "runLintCheck", {
    enumerable: true,
    get: function() {
        return runLintCheck;
    }
});
const _fs = require("fs");
const _picocolors = require("../picocolors");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _findup = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/find-up"));
const _semver = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/semver"));
const _commentjson = /*#__PURE__*/ _interop_require_wildcard(require("next/dist/compiled/comment-json"));
const _customFormatter = require("./customFormatter");
const _writeDefaultConfig = require("./writeDefaultConfig");
const _hasEslintConfiguration = require("./hasEslintConfiguration");
const _writeOutputFile = require("./writeOutputFile");
const _findpagesdir = require("../find-pages-dir");
const _installdependencies = require("../install-dependencies");
const _hasnecessarydependencies = require("../has-necessary-dependencies");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../../build/output/log"));
const _iserror = /*#__PURE__*/ _interop_require_wildcard(require("../is-error"));
const _getpkgmanager = require("../helpers/get-pkg-manager");
const _getESLintPromptValues = require("./getESLintPromptValues");
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
// 0 is off, 1 is warn, 2 is error. See https://eslint.org/docs/user-guide/configuring/rules#configuring-rules
const VALID_SEVERITY = [
    'off',
    'warn',
    'error'
];
function isValidSeverity(severity) {
    return VALID_SEVERITY.includes(severity);
}
const requiredPackages = [
    {
        file: 'eslint',
        pkg: 'eslint',
        exportsRestrict: false
    },
    {
        file: 'eslint-config-next',
        pkg: 'eslint-config-next',
        exportsRestrict: false
    }
];
async function cliPrompt(cwd) {
    console.log((0, _picocolors.bold)(`${(0, _picocolors.cyan)('?')} How would you like to configure ESLint? https://nextjs.org/docs/app/api-reference/config/eslint`));
    try {
        const cliSelect = (await Promise.resolve(require('next/dist/compiled/cli-select'))).default;
        const { value } = await cliSelect({
            values: await (0, _getESLintPromptValues.getESLintPromptValues)(cwd),
            valueRenderer: ({ title, recommended }, selected)=>{
                const name = selected ? (0, _picocolors.bold)((0, _picocolors.underline)((0, _picocolors.cyan)(title))) : title;
                return name + (recommended ? (0, _picocolors.bold)((0, _picocolors.yellow)(' (recommended)')) : '');
            },
            selected: (0, _picocolors.cyan)('❯ '),
            unselected: '  '
        });
        return {
            config: (value == null ? void 0 : value.config) ?? null
        };
    } catch  {
        return {
            config: null
        };
    }
}
async function lint(baseDir, lintDirs, eslintrcFile, pkgJsonPath, { lintDuringBuild = false, eslintOptions = null, reportErrorsOnly = false, maxWarnings = -1, formatter = null, outputFile = null }) {
    try {
        var _mod_CLIEngine, _ESLint_getErrorResults;
        // Load ESLint after we're sure it exists:
        const deps = await (0, _hasnecessarydependencies.hasNecessaryDependencies)(baseDir, requiredPackages);
        const packageManager = (0, _getpkgmanager.getPkgManager)(baseDir);
        if (deps.missing.some((dep)=>dep.pkg === 'eslint')) {
            _log.error(`ESLint must be installed${lintDuringBuild ? ' in order to run during builds:' : ':'} ${(0, _picocolors.bold)((0, _picocolors.cyan)((packageManager === 'yarn' ? 'yarn add --dev' : packageManager === 'pnpm' ? 'pnpm install --save-dev' : 'npm install --save-dev') + ' eslint'))}`);
            return null;
        }
        const mod = await Promise.resolve(require(deps.resolved.get('eslint')));
        // If V9 config was found, use flat config, or else use legacy.
        const useFlatConfig = eslintrcFile ? _path.default.basename(eslintrcFile).startsWith('eslint.config.') : false;
        let ESLint;
        // loadESLint is >= 8.57.0
        // PR https://github.com/eslint/eslint/pull/18098
        // Release https://github.com/eslint/eslint/releases/tag/v8.57.0
        if ('loadESLint' in mod) {
            // By default, configType is `flat`. If `useFlatConfig` is false, the return value is `LegacyESLint`.
            // https://github.com/eslint/eslint/blob/1def4cdfab1f067c5089df8b36242cdf912b0eb6/lib/types/index.d.ts#L1609-L1613
            ESLint = await mod.loadESLint({
                useFlatConfig
            });
        } else {
            // eslint < 8.57.0, use legacy ESLint
            ESLint = mod.ESLint;
        }
        let eslintVersion = (ESLint == null ? void 0 : ESLint.version) ?? ((_mod_CLIEngine = mod.CLIEngine) == null ? void 0 : _mod_CLIEngine.version);
        if (!eslintVersion || _semver.default.lt(eslintVersion, '7.0.0')) {
            return `${(0, _picocolors.red)('error')} - Your project has an older version of ESLint installed${eslintVersion ? ' (' + eslintVersion + ')' : ''}. Please upgrade to ESLint version 7 or above`;
        }
        let options = {
            useEslintrc: true,
            baseConfig: {},
            errorOnUnmatchedPattern: false,
            extensions: [
                '.js',
                '.jsx',
                '.ts',
                '.tsx'
            ],
            cache: true,
            ...eslintOptions
        };
        if (_semver.default.gte(eslintVersion, '9.0.0') && useFlatConfig) {
            for (const option of [
                'useEslintrc',
                'extensions',
                'ignorePath',
                'reportUnusedDisableDirectives',
                'resolvePluginsRelativeTo',
                'rulePaths',
                'inlineConfig',
                'maxWarnings'
            ]){
                if (option in options) {
                    delete options[option];
                }
            }
        }
        let eslint = new ESLint(options);
        let nextEslintPluginIsEnabled = false;
        const nextRulesEnabled = new Map();
        for (const configFile of [
            eslintrcFile,
            pkgJsonPath
        ]){
            if (!configFile) continue;
            const completeConfig = await eslint.calculateConfigForFile(configFile);
            if (!completeConfig) continue;
            const plugins = completeConfig.plugins;
            const hasNextPlugin = // in ESLint < 9, `plugins` value is string[]
            Array.isArray(plugins) ? plugins.includes('@next/next') : '@next/next' in plugins;
            if (hasNextPlugin) {
                nextEslintPluginIsEnabled = true;
                for (const [name, [severity]] of Object.entries(completeConfig.rules)){
                    if (!name.startsWith('@next/next/')) {
                        continue;
                    }
                    if (typeof severity === 'number' && severity >= 0 && severity < VALID_SEVERITY.length) {
                        nextRulesEnabled.set(name, VALID_SEVERITY[severity]);
                    } else if (typeof severity === 'string' && isValidSeverity(severity)) {
                        nextRulesEnabled.set(name, severity);
                    }
                }
                break;
            }
        }
        const pagesDir = (0, _findpagesdir.findPagesDir)(baseDir).pagesDir;
        const pagesDirRules = pagesDir ? [
            '@next/next/no-html-link-for-pages'
        ] : [];
        if (nextEslintPluginIsEnabled) {
            let updatedPagesDir = false;
            for (const rule of pagesDirRules){
                var _options_baseConfig_rules, _options_baseConfig_rules1;
                if (!((_options_baseConfig_rules = options.baseConfig.rules) == null ? void 0 : _options_baseConfig_rules[rule]) && !((_options_baseConfig_rules1 = options.baseConfig.rules) == null ? void 0 : _options_baseConfig_rules1[rule.replace('@next/next', '@next/babel-plugin-next')])) {
                    if (!options.baseConfig.rules) {
                        options.baseConfig.rules = {};
                    }
                    options.baseConfig.rules[rule] = [
                        1,
                        pagesDir
                    ];
                    updatedPagesDir = true;
                }
            }
            if (updatedPagesDir) {
                eslint = new ESLint(options);
            }
        } else {
            _log.warn('');
            _log.warn('The Next.js plugin was not detected in your ESLint configuration. See https://nextjs.org/docs/app/api-reference/config/eslint#migrating-existing-config');
        }
        const lintStart = process.hrtime();
        let results = await eslint.lintFiles(lintDirs);
        let selectedFormatter = null;
        if (options.fix) await ESLint.outputFixes(results);
        if (reportErrorsOnly) results = await ESLint.getErrorResults(results) // Only return errors if --quiet flag is used
        ;
        if (formatter) selectedFormatter = await eslint.loadFormatter(formatter);
        const formattedResult = await (0, _customFormatter.formatResults)(baseDir, results, selectedFormatter == null ? void 0 : selectedFormatter.format);
        const lintEnd = process.hrtime(lintStart);
        const totalWarnings = results.reduce((sum, file)=>sum + file.warningCount, 0);
        if (outputFile) await (0, _writeOutputFile.writeOutputFile)(outputFile, formattedResult.output);
        return {
            output: formattedResult.outputWithMessages,
            isError: ((_ESLint_getErrorResults = ESLint.getErrorResults(results)) == null ? void 0 : _ESLint_getErrorResults.length) > 0 || maxWarnings >= 0 && totalWarnings > maxWarnings,
            eventInfo: {
                durationInSeconds: lintEnd[0],
                eslintVersion: eslintVersion,
                lintedFilesCount: results.length,
                lintFix: !!options.fix,
                nextEslintPluginVersion: nextEslintPluginIsEnabled && deps.resolved.has('eslint-config-next') ? require(_path.default.join(_path.default.dirname(deps.resolved.get('eslint-config-next')), 'package.json')).version : null,
                nextEslintPluginErrorsCount: formattedResult.totalNextPluginErrorCount,
                nextEslintPluginWarningsCount: formattedResult.totalNextPluginWarningCount,
                nextRulesEnabled: Object.fromEntries(nextRulesEnabled)
            }
        };
    } catch (err) {
        if (lintDuringBuild) {
            _log.error(`ESLint: ${(0, _iserror.default)(err) && err.message ? err.message.replace(/\n/g, ' ') : err}`);
            return null;
        } else {
            throw (0, _iserror.getProperError)(err);
        }
    }
}
async function runLintCheck(baseDir, lintDirs, opts) {
    const { lintDuringBuild = false, eslintOptions = null, reportErrorsOnly = false, maxWarnings = -1, formatter = null, outputFile = null, strict = false } = opts;
    try {
        // Find user's .eslintrc file
        // See: https://eslint.org/docs/user-guide/configuring/configuration-files#configuration-file-formats
        const eslintrcFile = await (0, _findup.default)([
            // eslint v9
            'eslint.config.js',
            'eslint.config.mjs',
            'eslint.config.cjs',
            // TODO(jiwon): Support when it's stable.
            // TS extensions are experimental and requires to install another package `jiti`.
            // https://eslint.org/docs/latest/use/configure/configuration-files#typescript-configuration-files
            // 'eslint.config.ts',
            // 'eslint.config.mts',
            // 'eslint.config.cts',
            // eslint <= v8
            '.eslintrc.js',
            '.eslintrc.cjs',
            '.eslintrc.yaml',
            '.eslintrc.yml',
            '.eslintrc.json',
            '.eslintrc'
        ], {
            cwd: baseDir
        }) ?? null;
        const pkgJsonPath = await (0, _findup.default)('package.json', {
            cwd: baseDir
        }) ?? null;
        let packageJsonConfig = null;
        if (pkgJsonPath) {
            const pkgJsonContent = await _fs.promises.readFile(pkgJsonPath, {
                encoding: 'utf8'
            });
            packageJsonConfig = _commentjson.parse(pkgJsonContent);
        }
        const config = await (0, _hasEslintConfiguration.hasEslintConfiguration)(eslintrcFile, packageJsonConfig);
        let deps;
        if (config.exists) {
            // Run if ESLint config exists
            return await lint(baseDir, lintDirs, eslintrcFile, pkgJsonPath, {
                lintDuringBuild,
                eslintOptions,
                reportErrorsOnly,
                maxWarnings,
                formatter,
                outputFile
            });
        } else {
            // Display warning if no ESLint configuration is present inside
            // config file during "next build", no warning is shown when
            // no eslintrc file is present
            if (lintDuringBuild) {
                if (config.emptyPkgJsonConfig || config.emptyEslintrc) {
                    _log.warn(`No ESLint configuration detected. Run ${(0, _picocolors.bold)((0, _picocolors.cyan)('next lint'))} to begin setup`);
                }
                return null;
            } else {
                // Ask user what config they would like to start with for first time "next lint" setup
                const { config: selectedConfig } = strict ? await (0, _getESLintPromptValues.getESLintStrictValue)(baseDir) : await cliPrompt(baseDir);
                if (selectedConfig == null) {
                    // Show a warning if no option is selected in prompt
                    _log.warn('If you set up ESLint yourself, we recommend adding the Next.js ESLint plugin. See https://nextjs.org/docs/app/api-reference/config/eslint#migrating-existing-config');
                    return null;
                } else {
                    // Check if necessary deps installed, and install any that are missing
                    deps = await (0, _hasnecessarydependencies.hasNecessaryDependencies)(baseDir, requiredPackages);
                    if (deps.missing.length > 0) {
                        deps.missing.forEach((dep)=>{
                            if (dep.pkg === 'eslint') {
                                // pin to v9 to avoid breaking changes
                                dep.pkg = 'eslint@^9';
                            }
                        });
                        await (0, _installdependencies.installDependencies)(baseDir, deps.missing, true);
                    }
                    // Write default ESLint config.
                    // Check for /pages and src/pages is to make sure this happens in Next.js folder
                    if ([
                        'app',
                        'src/app',
                        'pages',
                        'src/pages'
                    ].some((dir)=>(0, _fs.existsSync)(_path.default.join(baseDir, dir)))) {
                        await (0, _writeDefaultConfig.writeDefaultConfig)(baseDir, config, selectedConfig, eslintrcFile, pkgJsonPath, packageJsonConfig);
                    }
                }
                _log.ready(`ESLint has successfully been configured. Run ${(0, _picocolors.bold)((0, _picocolors.cyan)('next lint'))} again to view warnings and errors.`);
                return null;
            }
        }
    } catch (err) {
        throw err;
    }
}

//# sourceMappingURL=runLintCheck.js.map