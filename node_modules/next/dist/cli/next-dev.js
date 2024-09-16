#!/usr/bin/env node
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "nextDev", {
    enumerable: true,
    get: function() {
        return nextDev;
    }
});
require("../server/lib/cpu-profile");
const _utils = require("../server/lib/utils");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../build/output/log"));
const _getprojectdir = require("../lib/get-project-dir");
const _constants = require("../shared/lib/constants");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _shared = require("../trace/shared");
const _storage = require("../telemetry/storage");
const _config = /*#__PURE__*/ _interop_require_default(require("../server/config"));
const _findpagesdir = require("../lib/find-pages-dir");
const _fileexists = require("../lib/file-exists");
const _getnpxcommand = require("../lib/helpers/get-npx-command");
const _mkcert = require("../lib/mkcert");
const _uploadtrace = /*#__PURE__*/ _interop_require_default(require("../trace/upload-trace"));
const _env = require("@next/env");
const _child_process = require("child_process");
const _getreservedport = require("../lib/helpers/get-reserved-port");
const _os = /*#__PURE__*/ _interop_require_default(require("os"));
const _nodeevents = require("node:events");
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
let dir;
let child;
let config;
let isTurboSession = false;
let traceUploadUrl;
let sessionStopHandled = false;
let sessionStarted = Date.now();
const handleSessionStop = async (signal)=>{
    if (child == null ? void 0 : child.pid) child.kill(signal ?? 0);
    if (sessionStopHandled) return;
    sessionStopHandled = true;
    if ((child == null ? void 0 : child.pid) && child.exitCode === null && child.signalCode === null) {
        await (0, _nodeevents.once)(child, "exit").catch(()=>{});
    }
    try {
        const { eventCliSessionStopped } = require("../telemetry/events/session-stopped");
        config = config || await (0, _config.default)(_constants.PHASE_DEVELOPMENT_SERVER, dir);
        let telemetry = _shared.traceGlobals.get("telemetry") || new _storage.Telemetry({
            distDir: _path.default.join(dir, config.distDir)
        });
        let pagesDir = !!_shared.traceGlobals.get("pagesDir");
        let appDir = !!_shared.traceGlobals.get("appDir");
        if (typeof _shared.traceGlobals.get("pagesDir") === "undefined" || typeof _shared.traceGlobals.get("appDir") === "undefined") {
            const pagesResult = (0, _findpagesdir.findPagesDir)(dir);
            appDir = !!pagesResult.appDir;
            pagesDir = !!pagesResult.pagesDir;
        }
        telemetry.record(eventCliSessionStopped({
            cliCommand: "dev",
            turboFlag: isTurboSession,
            durationMilliseconds: Date.now() - sessionStarted,
            pagesDir,
            appDir
        }), true);
        telemetry.flushDetached("dev", dir);
    } catch (_) {
    // errors here aren't actionable so don't add
    // noise to the output
    }
    if (traceUploadUrl) {
        (0, _uploadtrace.default)({
            traceUploadUrl,
            mode: "dev",
            projectDir: dir,
            distDir: config.distDir
        });
    }
    // ensure we re-enable the terminal cursor before exiting
    // the program, or the cursor could remain hidden
    process.stdout.write("\x1b[?25h");
    process.stdout.write("\n");
    process.exit(0);
};
process.on("SIGINT", ()=>handleSessionStop("SIGKILL"));
process.on("SIGTERM", ()=>handleSessionStop("SIGKILL"));
// exit event must be synchronous
process.on("exit", ()=>child == null ? void 0 : child.kill("SIGKILL"));
const nextDev = async (options, portSource, directory)=>{
    dir = (0, _getprojectdir.getProjectDir)(process.env.NEXT_PRIVATE_DEV_DIR || directory);
    // Check if pages dir exists and warn if not
    if (!await (0, _fileexists.fileExists)(dir, _fileexists.FileType.Directory)) {
        (0, _utils.printAndExit)(`> No such directory exists as the project root: ${dir}`);
    }
    async function preflight(skipOnReboot) {
        const { getPackageVersion, getDependencies } = await Promise.resolve(require("../lib/get-package-version"));
        const [sassVersion, nodeSassVersion] = await Promise.all([
            getPackageVersion({
                cwd: dir,
                name: "sass"
            }),
            getPackageVersion({
                cwd: dir,
                name: "node-sass"
            })
        ]);
        if (sassVersion && nodeSassVersion) {
            _log.warn("Your project has both `sass` and `node-sass` installed as dependencies, but should only use one or the other. " + "Please remove the `node-sass` dependency from your project. " + " Read more: https://nextjs.org/docs/messages/duplicate-sass");
        }
        if (!skipOnReboot) {
            const { dependencies, devDependencies } = await getDependencies({
                cwd: dir
            });
            // Warn if @next/font is installed as a dependency. Ignore `workspace:*` to not warn in the Next.js monorepo.
            if (dependencies["@next/font"] || devDependencies["@next/font"] && devDependencies["@next/font"] !== "workspace:*") {
                const command = (0, _getnpxcommand.getNpxCommand)(dir);
                _log.warn("Your project has `@next/font` installed as a dependency, please use the built-in `next/font` instead. " + "The `@next/font` package will be removed in Next.js 14. " + `You can migrate by running \`${command} @next/codemod@latest built-in-next-font .\`. Read more: https://nextjs.org/docs/messages/built-in-next-font`);
            }
        }
    }
    const port = options.port;
    if ((0, _getreservedport.isPortIsReserved)(port)) {
        (0, _utils.printAndExit)((0, _getreservedport.getReservedPortExplanation)(port), 1);
    }
    // If neither --port nor PORT were specified, it's okay to retry new ports.
    const allowRetry = portSource === "default";
    // We do not set a default host value here to prevent breaking
    // some set-ups that rely on listening on other interfaces
    const host = options.hostname;
    config = await (0, _config.default)(_constants.PHASE_DEVELOPMENT_SERVER, dir);
    if (options.experimentalUploadTrace && !process.env.NEXT_TRACE_UPLOAD_DISABLED) {
        traceUploadUrl = options.experimentalUploadTrace;
    }
    // TODO: remove in the next major version
    if (config.analyticsId) {
        _log.warn(`\`config.analyticsId\` is deprecated and will be removed in next major version. Read more: https://nextjs.org/docs/messages/deprecated-analyticsid`);
    }
    const devServerOptions = {
        dir,
        port,
        allowRetry,
        isDev: true,
        hostname: host
    };
    if (options.turbo) {
        process.env.TURBOPACK = "1";
    }
    isTurboSession = !!process.env.TURBOPACK;
    const distDir = _path.default.join(dir, config.distDir ?? ".next");
    (0, _shared.setGlobal)("phase", _constants.PHASE_DEVELOPMENT_SERVER);
    (0, _shared.setGlobal)("distDir", distDir);
    const startServerPath = require.resolve("../server/lib/start-server");
    async function startServer(startServerOptions) {
        return new Promise((resolve)=>{
            let resolved = false;
            const defaultEnv = _env.initialEnv || process.env;
            let NODE_OPTIONS = (0, _utils.getNodeOptionsWithoutInspect)();
            let nodeDebugType = (0, _utils.checkNodeDebugType)();
            const maxOldSpaceSize = (0, _utils.getMaxOldSpaceSize)();
            if (!maxOldSpaceSize && !process.env.NEXT_DISABLE_MEM_OVERRIDE) {
                const totalMem = _os.default.totalmem();
                const totalMemInMB = Math.floor(totalMem / 1024 / 1024);
                NODE_OPTIONS = `${NODE_OPTIONS} --max-old-space-size=${Math.floor(totalMemInMB * 0.5)}`;
            }
            if (nodeDebugType) {
                NODE_OPTIONS = `${NODE_OPTIONS} --${nodeDebugType}=${(0, _utils.getDebugPort)() + 1}`;
            }
            child = (0, _child_process.fork)(startServerPath, {
                stdio: "inherit",
                env: {
                    ...defaultEnv,
                    TURBOPACK: process.env.TURBOPACK,
                    NEXT_PRIVATE_WORKER: "1",
                    NODE_EXTRA_CA_CERTS: startServerOptions.selfSignedCertificate ? startServerOptions.selfSignedCertificate.rootCA : defaultEnv.NODE_EXTRA_CA_CERTS,
                    NODE_OPTIONS
                }
            });
            child.on("message", (msg)=>{
                if (msg && typeof msg === "object") {
                    if (msg.nextWorkerReady) {
                        child == null ? void 0 : child.send({
                            nextWorkerOptions: startServerOptions
                        });
                    } else if (msg.nextServerReady && !resolved) {
                        resolved = true;
                        resolve();
                    }
                }
            });
            child.on("exit", async (code, signal)=>{
                if (sessionStopHandled || signal) {
                    return;
                }
                if (code === _utils.RESTART_EXIT_CODE) {
                    // Starting the dev server will overwrite the `.next/trace` file, so we
                    // must upload the existing contents before restarting the server to
                    // preserve the metrics.
                    if (traceUploadUrl) {
                        (0, _uploadtrace.default)({
                            traceUploadUrl,
                            mode: "dev",
                            projectDir: dir,
                            distDir: config.distDir,
                            sync: true
                        });
                    }
                    return startServer(startServerOptions);
                }
                await handleSessionStop(signal);
            });
        });
    }
    const runDevServer = async (reboot)=>{
        try {
            if (!!options.experimentalHttps) {
                _log.warn("Self-signed certificates are currently an experimental feature, use with caution.");
                let certificate;
                const key = options.experimentalHttpsKey;
                const cert = options.experimentalHttpsCert;
                const rootCA = options.experimentalHttpsCa;
                if (key && cert) {
                    certificate = {
                        key: _path.default.resolve(key),
                        cert: _path.default.resolve(cert),
                        rootCA: rootCA ? _path.default.resolve(rootCA) : undefined
                    };
                } else {
                    certificate = await (0, _mkcert.createSelfSignedCertificate)(host);
                }
                await startServer({
                    ...devServerOptions,
                    selfSignedCertificate: certificate
                });
            } else {
                await startServer(devServerOptions);
            }
            await preflight(reboot);
        } catch (err) {
            console.error(err);
            process.exit(1);
        }
    };
    await runDevServer(false);
};

//# sourceMappingURL=next-dev.js.map