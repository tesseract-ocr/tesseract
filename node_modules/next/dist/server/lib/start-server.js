"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getRequestHandlers: null,
    startServer: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getRequestHandlers: function() {
        return getRequestHandlers;
    },
    startServer: function() {
        return startServer;
    }
});
const _getnetworkhost = require("../../lib/get-network-host");
require("../next");
require("../require-hook");
const _fs = /*#__PURE__*/ _interop_require_default(require("fs"));
const _v8 = /*#__PURE__*/ _interop_require_default(require("v8"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _http = /*#__PURE__*/ _interop_require_default(require("http"));
const _https = /*#__PURE__*/ _interop_require_default(require("https"));
const _os = /*#__PURE__*/ _interop_require_default(require("os"));
const _watchpack = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/watchpack"));
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../../build/output/log"));
const _debug = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/debug"));
const _utils = require("./utils");
const _formathostname = require("./format-hostname");
const _routerserver = require("./router-server");
const _constants = require("../../shared/lib/constants");
const _appinfolog = require("./app-info-log");
const _turbopackwarning = require("../../lib/turbopack-warning");
const _trace = require("../../trace");
const _ispostpone = require("./router-utils/is-postpone");
const _isipv6 = require("./is-ipv6");
const _asynccallbackset = require("./async-callback-set");
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
if (performance.getEntriesByName('next-start').length === 0) {
    performance.mark('next-start');
}
const debug = (0, _debug.default)('next:start-server');
let startServerSpan;
async function getRequestHandlers({ dir, port, isDev, onDevServerCleanup, server, hostname, minimalMode, keepAliveTimeout, experimentalHttpsServer, quiet }) {
    return (0, _routerserver.initialize)({
        dir,
        port,
        hostname,
        onDevServerCleanup,
        dev: isDev,
        minimalMode,
        server,
        keepAliveTimeout,
        experimentalHttpsServer,
        startServerSpan,
        quiet
    });
}
async function startServer(serverOptions) {
    const { dir, isDev, hostname, minimalMode, allowRetry, keepAliveTimeout, selfSignedCertificate } = serverOptions;
    let { port } = serverOptions;
    process.title = `next-server (v${"15.1.0"})`;
    let handlersReady = ()=>{};
    let handlersError = ()=>{};
    let handlersPromise = new Promise((resolve, reject)=>{
        handlersReady = resolve;
        handlersError = reject;
    });
    let requestHandler = async (req, res)=>{
        if (handlersPromise) {
            await handlersPromise;
            return requestHandler(req, res);
        }
        throw new Error('Invariant request handler was not setup');
    };
    let upgradeHandler = async (req, socket, head)=>{
        if (handlersPromise) {
            await handlersPromise;
            return upgradeHandler(req, socket, head);
        }
        throw new Error('Invariant upgrade handler was not setup');
    };
    let nextServer;
    // setup server listener as fast as possible
    if (selfSignedCertificate && !isDev) {
        throw new Error('Using a self signed certificate is only supported with `next dev`.');
    }
    async function requestListener(req, res) {
        try {
            if (handlersPromise) {
                await handlersPromise;
                handlersPromise = undefined;
            }
            await requestHandler(req, res);
        } catch (err) {
            res.statusCode = 500;
            res.end('Internal Server Error');
            _log.error(`Failed to handle request for ${req.url}`);
            console.error(err);
        } finally{
            if (isDev) {
                if (_v8.default.getHeapStatistics().used_heap_size > 0.8 * _v8.default.getHeapStatistics().heap_size_limit) {
                    _log.warn(`Server is approaching the used memory threshold, restarting...`);
                    (0, _trace.trace)('server-restart-close-to-memory-threshold', undefined, {
                        'memory.heapSizeLimit': String(_v8.default.getHeapStatistics().heap_size_limit),
                        'memory.heapUsed': String(_v8.default.getHeapStatistics().used_heap_size)
                    }).stop();
                    await (0, _trace.flushAllTraces)();
                    process.exit(_utils.RESTART_EXIT_CODE);
                }
            }
        }
    }
    const server = selfSignedCertificate ? _https.default.createServer({
        key: _fs.default.readFileSync(selfSignedCertificate.key),
        cert: _fs.default.readFileSync(selfSignedCertificate.cert)
    }, requestListener) : _http.default.createServer(requestListener);
    if (keepAliveTimeout) {
        server.keepAliveTimeout = keepAliveTimeout;
    }
    server.on('upgrade', async (req, socket, head)=>{
        try {
            await upgradeHandler(req, socket, head);
        } catch (err) {
            socket.destroy();
            _log.error(`Failed to handle request for ${req.url}`);
            console.error(err);
        }
    });
    let portRetryCount = 0;
    server.on('error', (err)=>{
        if (allowRetry && port && isDev && err.code === 'EADDRINUSE' && portRetryCount < 10) {
            _log.warn(`Port ${port} is in use, trying ${port + 1} instead.`);
            port += 1;
            portRetryCount += 1;
            server.listen(port, hostname);
        } else {
            _log.error(`Failed to start server`);
            console.error(err);
            process.exit(1);
        }
    });
    let cleanupListeners = isDev ? new _asynccallbackset.AsyncCallbackSet() : undefined;
    await new Promise((resolve)=>{
        server.on('listening', async ()=>{
            const nodeDebugType = (0, _utils.getNodeDebugType)();
            const addr = server.address();
            const actualHostname = (0, _formathostname.formatHostname)(typeof addr === 'object' ? (addr == null ? void 0 : addr.address) || hostname || 'localhost' : addr);
            const formattedHostname = !hostname || actualHostname === '0.0.0.0' ? 'localhost' : actualHostname === '[::]' ? '[::1]' : (0, _formathostname.formatHostname)(hostname);
            port = typeof addr === 'object' ? (addr == null ? void 0 : addr.port) || port : port;
            const networkHostname = hostname ?? (0, _getnetworkhost.getNetworkHost)((0, _isipv6.isIPv6)(actualHostname) ? 'IPv6' : 'IPv4');
            const protocol = selfSignedCertificate ? 'https' : 'http';
            const networkUrl = networkHostname ? `${protocol}://${(0, _formathostname.formatHostname)(networkHostname)}:${port}` : null;
            const appUrl = `${protocol}://${formattedHostname}:${port}`;
            if (nodeDebugType) {
                const formattedDebugAddress = (0, _utils.getFormattedDebugAddress)();
                _log.info(`the --${nodeDebugType} option was detected, the Next.js router server should be inspected at ${formattedDebugAddress}.`);
            }
            // Store the selected port to:
            // - expose it to render workers
            // - re-use it for automatic dev server restarts with a randomly selected port
            process.env.PORT = port + '';
            process.env.__NEXT_PRIVATE_ORIGIN = appUrl;
            // Only load env and config in dev to for logging purposes
            let envInfo;
            let expFeatureInfo;
            if (isDev) {
                const startServerInfo = await (0, _appinfolog.getStartServerInfo)(dir, isDev);
                envInfo = startServerInfo.envInfo;
                expFeatureInfo = startServerInfo.expFeatureInfo;
            }
            (0, _appinfolog.logStartInfo)({
                networkUrl,
                appUrl,
                envInfo,
                expFeatureInfo,
                maxExperimentalFeatures: 3
            });
            _log.event(`Starting...`);
            try {
                let cleanupStarted = false;
                const cleanup = ()=>{
                    if (cleanupStarted) {
                        // We can get duplicate signals, e.g. when `ctrl+c` is used in an
                        // interactive shell (i.e. bash, zsh), the shell will recursively
                        // send SIGINT to children. The parent `next-dev` process will also
                        // send us SIGINT.
                        return;
                    }
                    cleanupStarted = true;
                    (async ()=>{
                        debug('start-server process cleanup');
                        // first, stop accepting new connections and finish pending requests,
                        // because they might affect `nextServer.close()` (e.g. by scheduling an `after`)
                        await new Promise((res)=>server.close((err)=>{
                                if (err) console.error(err);
                                res();
                            }));
                        // now that no new requests can come in, clean up the rest
                        await Promise.all([
                            nextServer == null ? void 0 : nextServer.close().catch(console.error),
                            cleanupListeners == null ? void 0 : cleanupListeners.runAll().catch(console.error)
                        ]);
                        debug('start-server process cleanup finished');
                        process.exit(0);
                    })();
                };
                const exception = (err)=>{
                    if ((0, _ispostpone.isPostpone)(err)) {
                        // React postpones that are unhandled might end up logged here but they're
                        // not really errors. They're just part of rendering.
                        return;
                    }
                    // This is the render worker, we keep the process alive
                    console.error(err);
                };
                // Make sure commands gracefully respect termination signals (e.g. from Docker)
                // Allow the graceful termination to be manually configurable
                if (!process.env.NEXT_MANUAL_SIG_HANDLE) {
                    process.on('SIGINT', cleanup);
                    process.on('SIGTERM', cleanup);
                }
                process.on('rejectionHandled', ()=>{
                // It is ok to await a Promise late in Next.js as it allows for better
                // prefetching patterns to avoid waterfalls. We ignore loggining these.
                // We should've already errored in anyway unhandledRejection.
                });
                process.on('uncaughtException', exception);
                process.on('unhandledRejection', exception);
                const initResult = await getRequestHandlers({
                    dir,
                    port,
                    isDev,
                    onDevServerCleanup: cleanupListeners ? cleanupListeners.add.bind(cleanupListeners) : undefined,
                    server,
                    hostname,
                    minimalMode,
                    keepAliveTimeout,
                    experimentalHttpsServer: !!selfSignedCertificate
                });
                requestHandler = initResult.requestHandler;
                upgradeHandler = initResult.upgradeHandler;
                nextServer = initResult.server;
                const startServerProcessDuration = performance.mark('next-start-end') && performance.measure('next-start-duration', 'next-start', 'next-start-end').duration;
                handlersReady();
                const formatDurationText = startServerProcessDuration > 2000 ? `${Math.round(startServerProcessDuration / 100) / 10}s` : `${Math.round(startServerProcessDuration)}ms`;
                _log.event(`Ready in ${formatDurationText}`);
                if (process.env.TURBOPACK) {
                    await (0, _turbopackwarning.validateTurboNextConfig)({
                        dir: serverOptions.dir,
                        isDev: true
                    });
                }
            } catch (err) {
                // fatal error if we can't setup
                handlersError();
                console.error(err);
                process.exit(1);
            }
            resolve();
        });
        server.listen(port, hostname);
    });
    if (isDev) {
        function watchConfigFiles(dirToWatch, onChange) {
            const wp = new _watchpack.default();
            wp.watch({
                files: _constants.CONFIG_FILES.map((file)=>_path.default.join(dirToWatch, file))
            });
            wp.on('change', onChange);
        }
        watchConfigFiles(dir, async (filename)=>{
            if (process.env.__NEXT_DISABLE_MEMORY_WATCHER) {
                _log.info(`Detected change, manual restart required due to '__NEXT_DISABLE_MEMORY_WATCHER' usage`);
                return;
            }
            _log.warn(`Found a change in ${_path.default.basename(filename)}. Restarting the server to apply the changes...`);
            process.exit(_utils.RESTART_EXIT_CODE);
        });
    }
}
if (process.env.NEXT_PRIVATE_WORKER && process.send) {
    process.addListener('message', async (msg)=>{
        if (msg && typeof msg && msg.nextWorkerOptions && process.send) {
            startServerSpan = (0, _trace.trace)('start-dev-server', undefined, {
                cpus: String(_os.default.cpus().length),
                platform: _os.default.platform(),
                'memory.freeMem': String(_os.default.freemem()),
                'memory.totalMem': String(_os.default.totalmem()),
                'memory.heapSizeLimit': String(_v8.default.getHeapStatistics().heap_size_limit)
            });
            await startServerSpan.traceAsyncFn(()=>startServer(msg.nextWorkerOptions));
            const memoryUsage = process.memoryUsage();
            startServerSpan.setAttribute('memory.rss', String(memoryUsage.rss));
            startServerSpan.setAttribute('memory.heapTotal', String(memoryUsage.heapTotal));
            startServerSpan.setAttribute('memory.heapUsed', String(memoryUsage.heapUsed));
            process.send({
                nextServerReady: true,
                port: process.env.PORT
            });
        }
    });
    process.send({
        nextWorkerReady: true
    });
}

//# sourceMappingURL=start-server.js.map