#!/usr/bin/env node
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "nextInfo", {
    enumerable: true,
    get: function() {
        return nextInfo;
    }
});
const _os = /*#__PURE__*/ _interop_require_default(require("os"));
const _child_process = /*#__PURE__*/ _interop_require_default(require("child_process"));
const _picocolors = require("../lib/picocolors");
const _constants = require("../shared/lib/constants");
const _config = /*#__PURE__*/ _interop_require_default(require("../server/config"));
const _getregistry = require("../lib/helpers/get-registry");
const _parseversioninfo = require("../server/dev/parse-version-info");
const _VersionStalenessInfo = require("../client/components/react-dev-overlay/internal/components/VersionStalenessInfo/VersionStalenessInfo");
const _log = require("../build/output/log");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function getPackageVersion(packageName) {
    try {
        return require(`${packageName}/package.json`).version;
    } catch  {
        return 'N/A';
    }
}
async function getNextConfig() {
    var _config_experimental;
    const config = await (0, _config.default)(_constants.PHASE_INFO, process.cwd());
    return {
        output: config.output ?? 'N/A',
        experimental: {
            useWasmBinary: (_config_experimental = config.experimental) == null ? void 0 : _config_experimental.useWasmBinary
        }
    };
}
/**
 * Returns the version of the specified binary, by supplying `--version` argument.
 * N/A if it fails to run the binary.
 */ function getBinaryVersion(binaryName) {
    try {
        return _child_process.default.execFileSync(binaryName, [
            '--version'
        ]).toString().trim();
    } catch  {
        try {
            return _child_process.default.execSync(`${binaryName} --version`).toString().trim();
        } catch  {
            return 'N/A';
        }
    }
}
/**
 * Collect basic next.js installation information and print it to stdout.
 */ async function printInfo() {
    const installedRelease = getPackageVersion('next');
    const nextConfig = await getNextConfig();
    let stalenessWithTitle = '';
    let title = '';
    let versionInfo;
    try {
        const registry = (0, _getregistry.getRegistry)();
        const res = await fetch(`${registry}-/package/next/dist-tags`);
        const tags = await res.json();
        versionInfo = (0, _parseversioninfo.parseVersionInfo)({
            installed: installedRelease,
            latest: tags.latest,
            canary: tags.canary
        });
        title = (0, _VersionStalenessInfo.getStaleness)(versionInfo).title;
        if (title) {
            stalenessWithTitle = ` // ${title}`;
        }
    } catch (e) {
        console.warn(`${(0, _picocolors.yellow)((0, _picocolors.bold)('warn'))}  - Failed to fetch latest canary version. (Reason: ${e.message}.)
      Detected "${installedRelease}". Visit https://github.com/vercel/next.js/releases.
      Make sure to try the latest canary version (eg.: \`npm install next@canary\`) to confirm the issue still exists before creating a new issue.
      \nLearn more: ${(0, _picocolors.cyan)('https://nextjs.org/docs/messages/opening-an-issue')}`);
    }
    const cpuCores = _os.default.cpus().length;
    console.log(`
Operating System:
  Platform: ${_os.default.platform()}
  Arch: ${_os.default.arch()}
  Version: ${_os.default.version()}
  Available memory (MB): ${Math.ceil(_os.default.totalmem() / 1024 / 1024)}
  Available CPU cores: ${cpuCores > 0 ? cpuCores : 'N/A'}
Binaries:
  Node: ${process.versions.node}
  npm: ${getBinaryVersion('npm')}
  Yarn: ${getBinaryVersion('yarn')}
  pnpm: ${getBinaryVersion('pnpm')}
Relevant Packages:
  next: ${installedRelease}${stalenessWithTitle}
  eslint-config-next: ${getPackageVersion('eslint-config-next')}
  react: ${getPackageVersion('react')}
  react-dom: ${getPackageVersion('react-dom')}
  typescript: ${getPackageVersion('typescript')}
Next.js Config:
  output: ${nextConfig.output}`);
    if (versionInfo == null ? void 0 : versionInfo.staleness.startsWith('stale')) {
        (0, _log.warn)(`${title}
   Please try the latest canary version (\`npm install next@canary\`) to confirm the issue still exists before creating a new issue.
   Read more - https://nextjs.org/docs/messages/opening-an-issue`);
    }
}
/**
 * Using system-installed tools per each platform, trying to read shared dependencies of next-swc.
 * This is mainly for debugging DLOPEN failure.
 *
 * We don't / can't install these tools by ourselves, will skip the check if we can't find them.
 */ async function runSharedDependencyCheck(tools, skipMessage) {
    var _getSupportedArchTriples_currentPlatform;
    const currentPlatform = _os.default.platform();
    const spawn = require('next/dist/compiled/cross-spawn');
    const { getSupportedArchTriples } = require('../build/swc');
    const triples = ((_getSupportedArchTriples_currentPlatform = getSupportedArchTriples()[currentPlatform]) == null ? void 0 : _getSupportedArchTriples_currentPlatform[_os.default.arch()]) ?? [];
    // First, check if system have a tool installed. We can't install these by our own.
    const availableTools = [];
    for (const tool of tools){
        try {
            const check = spawn.sync(tool.bin, tool.checkArgs);
            if (check.status === 0) {
                availableTools.push(tool);
            }
        } catch  {
        // ignore if existence check fails
        }
    }
    if (availableTools.length === 0) {
        return {
            messages: skipMessage,
            result: 'skipped'
        };
    }
    const outputs = [];
    let result = 'fail';
    for (const triple of triples){
        const triplePkgName = `@next/swc-${triple.platformArchABI}`;
        let resolved;
        try {
            resolved = require.resolve(triplePkgName);
        } catch (e) {
            return {
                messages: 'Cannot find next-swc installation, skipping dependencies check',
                result: 'skipped'
            };
        }
        for (const tool of availableTools){
            const proc = spawn(tool.bin, [
                ...tool.args,
                resolved
            ]);
            outputs.push(`Running ${tool.bin} ------------- `);
            // Captures output, doesn't matter if it fails or not since we'll forward both to output.
            const procPromise = new Promise((resolve)=>{
                proc.stdout.on('data', function(data) {
                    outputs.push(data);
                });
                proc.stderr.on('data', function(data) {
                    outputs.push(data);
                });
                proc.on('close', (c)=>resolve(c));
            });
            let code = await procPromise;
            if (code === 0) {
                result = 'pass';
            }
        }
    }
    return {
        output: outputs.join('\n'),
        result
    };
}
/**
 * Collect additional diagnostics information.
 */ async function printVerboseInfo() {
    const fs = require('fs');
    const currentPlatform = _os.default.platform();
    if (currentPlatform !== 'win32' && currentPlatform !== 'linux' && currentPlatform !== 'darwin') {
        console.log('Unsupported platform, only win32, linux, darwin are supported.');
        return;
    }
    // List of tasks to run.
    const tasks = [
        {
            title: 'Host system information',
            scripts: {
                default: async ()=>{
                    // Node.js diagnostic report contains basic information, i.e OS version, CPU architecture, etc.
                    // Only collect few addtional details here.
                    const isWsl = require('next/dist/compiled/is-wsl');
                    const ciInfo = require('next/dist/compiled/ci-info');
                    const isDocker = require('next/dist/compiled/is-docker');
                    const output = `
  WSL: ${isWsl}
  Docker: ${isDocker()}
  CI: ${ciInfo.isCI ? ciInfo.name || 'unknown' : 'false'}
`;
                    return {
                        output,
                        result: 'pass'
                    };
                }
            }
        },
        {
            title: 'Next.js installation',
            scripts: {
                default: async ()=>{
                    const installedRelease = getPackageVersion('next');
                    const nextConfig = await getNextConfig();
                    const output = `
  Binaries:
    Node: ${process.versions.node}
    npm: ${getBinaryVersion('npm')}
    Yarn: ${getBinaryVersion('yarn')}
    pnpm: ${getBinaryVersion('pnpm')}
  Relevant Packages:
    next: ${installedRelease}
    eslint-config-next: ${getPackageVersion('eslint-config-next')}
    react: ${getPackageVersion('react')}
    react-dom: ${getPackageVersion('react-dom')}
    typescript: ${getPackageVersion('typescript')}
  Next.js Config:
    output: ${nextConfig.output}

`;
                    return {
                        output,
                        result: 'pass'
                    };
                }
            }
        },
        {
            title: 'Node.js diagnostic report',
            scripts: {
                default: async ()=>{
                    var _process_report;
                    const report = (_process_report = process.report) == null ? void 0 : _process_report.getReport();
                    if (!report) {
                        return {
                            messages: 'Node.js diagnostic report is not available.',
                            result: 'fail'
                        };
                    }
                    const { header, javascriptHeap, sharedObjects } = report;
                    // Delete some fields potentially containing sensitive information.
                    header == null ? true : delete header.cwd;
                    header == null ? true : delete header.commandLine;
                    header == null ? true : delete header.host;
                    header == null ? true : delete header.cpus;
                    header == null ? true : delete header.networkInterfaces;
                    const reportSummary = {
                        header,
                        javascriptHeap,
                        sharedObjects
                    };
                    return {
                        output: JSON.stringify(reportSummary, null, 2),
                        result: 'pass'
                    };
                }
            }
        },
        {
            title: 'next-swc installation',
            scripts: {
                default: async ()=>{
                    var _platformArchTriples_currentPlatform;
                    const output = [];
                    // First, try to load next-swc via loadBindings.
                    try {
                        var _nextConfig_experimental;
                        let nextConfig = await getNextConfig();
                        const { loadBindings } = require('../build/swc');
                        const bindings = await loadBindings((_nextConfig_experimental = nextConfig.experimental) == null ? void 0 : _nextConfig_experimental.useWasmBinary);
                        // Run arbitrary function to verify the bindings are loaded correctly.
                        const target = bindings.getTargetTriple();
                        // We think next-swc is installed correctly if getTargetTriple returns.
                        return {
                            output: `next-swc is installed correctly for ${target}`,
                            result: 'pass'
                        };
                    } catch (e) {
                        output.push(`loadBindings() failed: ${e.message}`);
                    }
                    const { platformArchTriples } = require('next/dist/compiled/@napi-rs/triples');
                    const triples = (_platformArchTriples_currentPlatform = platformArchTriples[currentPlatform]) == null ? void 0 : _platformArchTriples_currentPlatform[_os.default.arch()];
                    if (!triples || triples.length === 0) {
                        return {
                            messages: `No target triples found for ${currentPlatform} / ${_os.default.arch()}`,
                            result: 'fail'
                        };
                    }
                    // Trying to manually resolve corresponding target triples to see if bindings are physically located.
                    const path = require('path');
                    let fallbackBindingsDirectory;
                    try {
                        const nextPath = path.dirname(require.resolve('next/package.json'));
                        fallbackBindingsDirectory = path.join(nextPath, 'next-swc-fallback');
                    } catch (e) {
                    // Not able to locate next package from current running location, skipping fallback bindings check.
                    }
                    const tryResolve = (pkgName)=>{
                        try {
                            const resolved = require.resolve(pkgName);
                            const fileExists = fs.existsSync(resolved);
                            let loadError;
                            let loadSuccess;
                            try {
                                loadSuccess = !!require(resolved).getTargetTriple();
                            } catch (e) {
                                loadError = e.message;
                            }
                            output.push(`${pkgName} exists: ${fileExists} for the triple ${loadSuccess}`);
                            if (loadError) {
                                output.push(`${pkgName} load failed: ${loadError ?? 'unknown'}`);
                            }
                            if (loadSuccess) {
                                return true;
                            }
                        } catch (e) {
                            output.push(`${pkgName} resolve failed: ${e.message ?? 'unknown'}`);
                        }
                        return false;
                    };
                    for (const triple of triples){
                        const triplePkgName = `@next/swc-${triple.platformArchABI}`;
                        // Check installed optional dependencies. This is the normal way package being installed.
                        // For the targets have multiple triples (gnu / musl), if any of them loads successfully, we consider as installed.
                        if (tryResolve(triplePkgName)) {
                            break;
                        }
                        // Check if fallback binaries are installed.
                        if (!fallbackBindingsDirectory) {
                            continue;
                        }
                        tryResolve(path.join(fallbackBindingsDirectory, triplePkgName));
                    }
                    return {
                        output: output.join('\n'),
                        result: 'pass'
                    };
                }
            }
        },
        {
            // For the simplicity, we only check the correctly installed optional dependencies -
            // as this is mainly for checking DLOPEN failure. If user hit MODULE_NOT_FOUND,
            // expect above next-swc installation would give some hint instead.
            title: 'next-swc shared object dependencies',
            scripts: {
                linux: async ()=>{
                    const skipMessage = 'This diagnostics uses system-installed tools (ldd) to check next-swc dependencies, but it is not found. Skipping dependencies check.';
                    return await runSharedDependencyCheck([
                        {
                            bin: 'ldd',
                            checkArgs: [
                                '--help'
                            ],
                            args: [
                                '--verbose'
                            ]
                        }
                    ], skipMessage);
                },
                win32: async ()=>{
                    const skipMessage = `This diagnostics uses system-installed tools (dumpbin.exe) to check next-swc dependencies, but it was not found in the path. Skipping dependencies check.
          dumpbin (https://learn.microsoft.com/en-us/cpp/build/reference/dumpbin-reference) is a part of Microsoft VC toolset,
          can be installed with Windows SDK, Windows Build tools or Visual Studio.

          Please make sure you have one of them installed and dumpbin.exe is in the path.
          `;
                    return await runSharedDependencyCheck([
                        {
                            bin: 'dumpbin.exe',
                            checkArgs: [
                                '/summary'
                            ],
                            args: [
                                '/imports'
                            ]
                        }
                    ], skipMessage);
                },
                darwin: async ()=>{
                    const skipMessage = 'This diagnostics uses system-installed tools (otools, dyld_info) to check next-swc dependencies, but none of them are found. Skipping dependencies check.';
                    return await runSharedDependencyCheck([
                        {
                            bin: 'otool',
                            checkArgs: [
                                '--version'
                            ],
                            args: [
                                '-L'
                            ]
                        },
                        {
                            bin: 'dyld_info',
                            checkArgs: [],
                            args: []
                        }
                    ], skipMessage);
                }
            }
        }
    ];
    // Collected output after running all tasks.
    const report = [];
    console.log('\n');
    for (const task of tasks){
        if (task.targetPlatform && task.targetPlatform !== currentPlatform) {
            report.push({
                title: task.title,
                result: {
                    messages: undefined,
                    output: `[SKIPPED (${_os.default.platform()} / ${task.targetPlatform})] ${task.title}`,
                    result: 'skipped'
                }
            });
            continue;
        }
        const taskScript = task.scripts[currentPlatform] ?? task.scripts.default;
        let taskResult;
        try {
            taskResult = await taskScript();
        } catch (e) {
            taskResult = {
                messages: `Unexpected failure while running diagnostics: ${e.message}`,
                result: 'fail'
            };
        }
        console.log(`- ${task.title}: ${taskResult.result}`);
        if (taskResult.messages) {
            console.log(`  ${taskResult.messages}`);
        }
        report.push({
            title: task.title,
            result: taskResult
        });
    }
    console.log(`\n${(0, _picocolors.bold)('Generated diagnostics report')}`);
    console.log(`\nPlease copy below report and paste it into your issue.`);
    for (const { title, result } of report){
        console.log(`\n### ${title}`);
        if (result.messages) {
            console.log(result.messages);
        }
        if (result.output) {
            console.log(result.output);
        }
    }
}
/**
 * Runs few scripts to collect system information to help with debugging next.js installation issues.
 * There are 2 modes, by default it collects basic next.js installation with runtime information. If
 * `--verbose` mode is enabled it'll try to collect, verify more data for next-swc installation and others.
 */ const nextInfo = async (options)=>{
    if (options.verbose) {
        await printVerboseInfo();
    } else {
        await printInfo();
    }
};

//# sourceMappingURL=next-info.js.map