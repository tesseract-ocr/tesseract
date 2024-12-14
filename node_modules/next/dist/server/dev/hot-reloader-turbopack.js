"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "createHotReloaderTurbopack", {
    enumerable: true,
    get: function() {
        return createHotReloaderTurbopack;
    }
});
const _promises = require("fs/promises");
const _path = require("path");
const _url = require("url");
const _ws = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/ws"));
const _store = require("../../build/output/store");
const _hotreloadertypes = require("./hot-reloader-types");
const _swc = require("../../build/swc");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../../build/output/log"));
const _hotreloaderwebpack = require("./hot-reloader-webpack");
const _constants = require("../../shared/lib/constants");
const _middlewareturbopack = require("../../client/components/react-dev-overlay/server/middleware-turbopack");
const _utils = require("../../shared/lib/utils");
const _utils1 = require("../utils");
const _requirecache = require("./require-cache");
const _renderserver = require("../lib/render-server");
const _denormalizepagepath = require("../../shared/lib/page-path/denormalize-page-path");
const _trace = require("../../trace");
const _turbopackutils = require("./turbopack-utils");
const _setupdevbundler = require("../lib/router-utils/setup-dev-bundler");
const _manifestloader = require("./turbopack/manifest-loader");
const _ondemandentryhandler = require("./on-demand-entry-handler");
const _entrykey = require("./turbopack/entry-key");
const _messages = require("./messages");
const _encryptionutilsserver = require("../app-render/encryption-utils-server");
const _apppageroutedefinition = require("../route-definitions/app-page-route-definition");
const _apppaths = require("../../shared/lib/router/utils/app-paths");
const _utils2 = require("../lib/utils");
const _ismetadataroute = require("../../lib/metadata/is-metadata-route");
const _patcherrorinspect = require("../patch-error-inspect");
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
// import { getSupportedBrowsers } from '../../build/utils'
const wsServer = new _ws.default.Server({
    noServer: true
});
const isTestMode = !!(process.env.NEXT_TEST_MODE || process.env.__NEXT_TEST_MODE || process.env.DEBUG);
const sessionId = Math.floor(Number.MAX_SAFE_INTEGER * Math.random());
/**
 * Replaces turbopack://[project] with the specified project in the `source` field.
 */ function rewriteTurbopackSources(projectRoot, sourceMap) {
    if ('sections' in sourceMap) {
        for (const section of sourceMap.sections){
            rewriteTurbopackSources(projectRoot, section.map);
        }
    } else {
        for(let i = 0; i < sourceMap.sources.length; i++){
            sourceMap.sources[i] = (0, _url.pathToFileURL)((0, _path.join)(projectRoot, sourceMap.sources[i].replace(/turbopack:\/\/\[project\]/, ''))).toString();
        }
    }
}
function getSourceMapFromTurbopack(project, projectRoot, sourceURL) {
    let sourceMapJson = null;
    try {
        sourceMapJson = project.getSourceMapSync(sourceURL);
    } catch (err) {}
    if (sourceMapJson === null) {
        return undefined;
    } else {
        const payload = JSON.parse(sourceMapJson);
        // The sourcemap from Turbopack is not yet written to disk so its `sources`
        // are not absolute paths yet. We need to rewrite them to be absolute paths.
        rewriteTurbopackSources(projectRoot, payload);
        return payload;
    }
}
async function createHotReloaderTurbopack(opts, serverFields, distDir, resetFetch) {
    var _opts_nextConfig_experimental_turbo, _nextConfig_watchOptions, _opts_nextConfig_experimental_turbo1;
    const dev = true;
    const buildId = 'development';
    const { nextConfig, dir } = opts;
    const { loadBindings } = require('../../build/swc');
    let bindings = await loadBindings();
    // For the debugging purpose, check if createNext or equivalent next instance setup in test cases
    // works correctly. Normally `run-test` hides output so only will be visible when `--debug` flag is used.
    if (process.env.TURBOPACK && isTestMode) {
        require('console').log('Creating turbopack project', {
            dir,
            testMode: isTestMode
        });
    }
    const hasRewrites = opts.fsChecker.rewrites.afterFiles.length > 0 || opts.fsChecker.rewrites.beforeFiles.length > 0 || opts.fsChecker.rewrites.fallback.length > 0;
    const hotReloaderSpan = (0, _trace.trace)('hot-reloader', undefined, {
        version: "15.1.0"
    });
    // Ensure the hotReloaderSpan is flushed immediately as it's the parentSpan for all processing
    // of the current `next dev` invocation.
    hotReloaderSpan.stop();
    const encryptionKey = await (0, _encryptionutilsserver.generateEncryptionKeyBase64)({
        isBuild: false,
        distDir
    });
    // TODO: Implement
    let clientRouterFilters;
    if (nextConfig.experimental.clientRouterFilter) {
    // TODO this need to be set correctly for persistent caching to work
    }
    // const supportedBrowsers = await getSupportedBrowsers(dir, dev)
    const supportedBrowsers = [
        'last 1 Chrome versions, last 1 Firefox versions, last 1 Safari versions, last 1 Edge versions'
    ];
    const project = await bindings.turbo.createProject({
        projectPath: dir,
        rootPath: ((_opts_nextConfig_experimental_turbo = opts.nextConfig.experimental.turbo) == null ? void 0 : _opts_nextConfig_experimental_turbo.root) || opts.nextConfig.outputFileTracingRoot || dir,
        distDir,
        nextConfig: opts.nextConfig,
        jsConfig: await (0, _turbopackutils.getTurbopackJsConfig)(dir, nextConfig),
        watch: {
            enable: dev,
            pollIntervalMs: (_nextConfig_watchOptions = nextConfig.watchOptions) == null ? void 0 : _nextConfig_watchOptions.pollIntervalMs
        },
        dev,
        env: process.env,
        defineEnv: (0, _swc.createDefineEnv)({
            isTurbopack: true,
            clientRouterFilters,
            config: nextConfig,
            dev,
            distDir,
            fetchCacheKeyPrefix: opts.nextConfig.experimental.fetchCacheKeyPrefix,
            hasRewrites,
            // TODO: Implement
            middlewareMatchers: undefined
        }),
        buildId,
        encryptionKey,
        previewProps: opts.fsChecker.prerenderManifest.preview,
        browserslistQuery: supportedBrowsers.join(', ')
    }, {
        persistentCaching: (0, _turbopackutils.isPersistentCachingEnabled)(opts.nextConfig),
        memoryLimit: (_opts_nextConfig_experimental_turbo1 = opts.nextConfig.experimental.turbo) == null ? void 0 : _opts_nextConfig_experimental_turbo1.memoryLimit
    });
    (0, _patcherrorinspect.setBundlerFindSourceMapImplementation)(getSourceMapFromTurbopack.bind(null, project, dir));
    opts.onDevServerCleanup == null ? void 0 : opts.onDevServerCleanup.call(opts, async ()=>{
        (0, _patcherrorinspect.setBundlerFindSourceMapImplementation)(()=>undefined);
        await project.onExit();
    });
    const entrypointsSubscription = project.entrypointsSubscribe();
    const currentWrittenEntrypoints = new Map();
    const currentEntrypoints = {
        global: {
            app: undefined,
            document: undefined,
            error: undefined,
            middleware: undefined,
            instrumentation: undefined
        },
        page: new Map(),
        app: new Map()
    };
    const currentTopLevelIssues = new Map();
    const currentEntryIssues = new Map();
    const manifestLoader = new _manifestloader.TurbopackManifestLoader({
        buildId,
        distDir,
        encryptionKey
    });
    // Dev specific
    const changeSubscriptions = new Map();
    const serverPathState = new Map();
    const readyIds = new Set();
    let currentEntriesHandlingResolve;
    let currentEntriesHandling = new Promise((resolve)=>currentEntriesHandlingResolve = resolve);
    const assetMapper = new _turbopackutils.AssetMapper();
    function clearRequireCache(key, writtenEndpoint, { force } = {}) {
        if (force) {
            for (const { path, contentHash } of writtenEndpoint.serverPaths){
                serverPathState.set(path, contentHash);
            }
        } else {
            // Figure out if the server files have changed
            let hasChange = false;
            for (const { path, contentHash } of writtenEndpoint.serverPaths){
                // We ignore source maps
                if (path.endsWith('.map')) continue;
                const localKey = `${key}:${path}`;
                const localHash = serverPathState.get(localKey);
                const globalHash = serverPathState.get(path);
                if (localHash && localHash !== contentHash || globalHash && globalHash !== contentHash) {
                    hasChange = true;
                    serverPathState.set(key, contentHash);
                    serverPathState.set(path, contentHash);
                } else {
                    if (!localHash) {
                        serverPathState.set(key, contentHash);
                    }
                    if (!globalHash) {
                        serverPathState.set(path, contentHash);
                    }
                }
            }
            if (!hasChange) {
                return;
            }
        }
        resetFetch();
        const hasAppPaths = writtenEndpoint.serverPaths.some(({ path: p })=>p.startsWith('server/app'));
        if (hasAppPaths) {
            (0, _requirecache.deleteAppClientCache)();
        }
        const serverPaths = writtenEndpoint.serverPaths.map(({ path: p })=>(0, _path.join)(distDir, p));
        for (const file of serverPaths){
            (0, _renderserver.clearModuleContext)(file);
            (0, _requirecache.deleteCache)(file);
        }
        return;
    }
    const buildingIds = new Set();
    const startBuilding = (id, requestUrl, forceRebuild)=>{
        if (!forceRebuild && readyIds.has(id)) {
            return ()=>{};
        }
        if (buildingIds.size === 0) {
            _store.store.setState({
                loading: true,
                trigger: id,
                url: requestUrl
            }, true);
        }
        buildingIds.add(id);
        return function finishBuilding() {
            if (buildingIds.size === 0) {
                return;
            }
            readyIds.add(id);
            buildingIds.delete(id);
            if (buildingIds.size === 0) {
                hmrEventHappened = false;
                _store.store.setState({
                    loading: false
                }, true);
            }
        };
    };
    let hmrEventHappened = false;
    let hmrHash = 0;
    const clients = new Set();
    const clientStates = new WeakMap();
    function sendToClient(client, payload) {
        client.send(JSON.stringify(payload));
    }
    function sendEnqueuedMessages() {
        for (const [, issueMap] of currentEntryIssues){
            if ([
                ...issueMap.values()
            ].filter((i)=>i.severity !== 'warning').length > 0) {
                // During compilation errors we want to delay the HMR events until errors are fixed
                return;
            }
        }
        for (const client of clients){
            const state = clientStates.get(client);
            if (!state) {
                continue;
            }
            for (const [, issueMap] of state.clientIssues){
                if ([
                    ...issueMap.values()
                ].filter((i)=>i.severity !== 'warning').length > 0) {
                    // During compilation errors we want to delay the HMR events until errors are fixed
                    return;
                }
            }
            for (const payload of state.hmrPayloads.values()){
                sendToClient(client, payload);
            }
            state.hmrPayloads.clear();
            if (state.turbopackUpdates.length > 0) {
                sendToClient(client, {
                    action: _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.TURBOPACK_MESSAGE,
                    data: state.turbopackUpdates
                });
                state.turbopackUpdates.length = 0;
            }
        }
    }
    const sendEnqueuedMessagesDebounce = (0, _utils1.debounce)(sendEnqueuedMessages, 2);
    const sendHmr = (id, payload)=>{
        for (const client of clients){
            var _clientStates_get;
            (_clientStates_get = clientStates.get(client)) == null ? void 0 : _clientStates_get.hmrPayloads.set(id, payload);
        }
        hmrEventHappened = true;
        sendEnqueuedMessagesDebounce();
    };
    function sendTurbopackMessage(payload) {
        // TODO(PACK-2049): For some reason we end up emitting hundreds of issues messages on bigger apps,
        //   a lot of which are duplicates.
        //   They are currently not handled on the client at all, so might as well not send them for now.
        payload.diagnostics = [];
        payload.issues = [];
        for (const client of clients){
            var _clientStates_get;
            (_clientStates_get = clientStates.get(client)) == null ? void 0 : _clientStates_get.turbopackUpdates.push(payload);
        }
        hmrEventHappened = true;
        sendEnqueuedMessagesDebounce();
    }
    async function subscribeToChanges(key, includeIssues, endpoint, makePayload, onError) {
        if (changeSubscriptions.has(key)) {
            return;
        }
        const { side } = (0, _entrykey.splitEntryKey)(key);
        const changedPromise = endpoint[`${side}Changed`](includeIssues);
        changeSubscriptions.set(key, changedPromise);
        try {
            const changed = await changedPromise;
            for await (const change of changed){
                (0, _turbopackutils.processIssues)(currentEntryIssues, key, change, false, true);
                const payload = await makePayload(change);
                if (payload) {
                    sendHmr(key, payload);
                }
            }
        } catch (e) {
            changeSubscriptions.delete(key);
            const payload = await (onError == null ? void 0 : onError(e));
            if (payload) {
                sendHmr(key, payload);
            }
            return;
        }
        changeSubscriptions.delete(key);
    }
    async function unsubscribeFromChanges(key) {
        const subscription = await changeSubscriptions.get(key);
        if (subscription) {
            await (subscription.return == null ? void 0 : subscription.return.call(subscription));
            changeSubscriptions.delete(key);
        }
        currentEntryIssues.delete(key);
    }
    async function subscribeToHmrEvents(client, id) {
        const key = (0, _entrykey.getEntryKey)('assets', 'client', id);
        if (!(0, _turbopackutils.hasEntrypointForKey)(currentEntrypoints, key, assetMapper)) {
            // maybe throw an error / force the client to reload?
            return;
        }
        const state = clientStates.get(client);
        if (!state || state.subscriptions.has(id)) {
            return;
        }
        const subscription = project.hmrEvents(id);
        state.subscriptions.set(id, subscription);
        // The subscription will always emit once, which is the initial
        // computation. This is not a change, so swallow it.
        try {
            await subscription.next();
            for await (const data of subscription){
                (0, _turbopackutils.processIssues)(state.clientIssues, key, data, false, true);
                if (data.type !== 'issues') {
                    sendTurbopackMessage(data);
                }
            }
        } catch (e) {
            // The client might be using an HMR session from a previous server, tell them
            // to fully reload the page to resolve the issue. We can't use
            // `hotReloader.send` since that would force every connected client to
            // reload, only this client is out of date.
            const reloadAction = {
                action: _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.RELOAD_PAGE,
                data: `error in HMR event subscription for ${id}: ${e}`
            };
            sendToClient(client, reloadAction);
            client.close();
            return;
        }
    }
    function unsubscribeFromHmrEvents(client, id) {
        const state = clientStates.get(client);
        if (!state) {
            return;
        }
        const subscription = state.subscriptions.get(id);
        subscription == null ? void 0 : subscription.return();
        const key = (0, _entrykey.getEntryKey)('assets', 'client', id);
        state.clientIssues.delete(key);
    }
    async function handleEntrypointsSubscription() {
        for await (const entrypoints of entrypointsSubscription){
            if (!currentEntriesHandlingResolve) {
                currentEntriesHandling = new Promise(// eslint-disable-next-line no-loop-func
                (resolve)=>currentEntriesHandlingResolve = resolve);
            }
            (0, _turbopackutils.processTopLevelIssues)(currentTopLevelIssues, entrypoints);
            await (0, _turbopackutils.handleEntrypoints)({
                entrypoints,
                currentEntrypoints,
                currentEntryIssues,
                manifestLoader,
                devRewrites: opts.fsChecker.rewrites,
                productionRewrites: undefined,
                logErrors: true,
                dev: {
                    assetMapper,
                    changeSubscriptions,
                    clients,
                    clientStates,
                    serverFields,
                    hooks: {
                        handleWrittenEndpoint: (id, result)=>{
                            currentWrittenEntrypoints.set(id, result);
                            clearRequireCache(id, result);
                        },
                        propagateServerField: _setupdevbundler.propagateServerField.bind(null, opts),
                        sendHmr,
                        startBuilding,
                        subscribeToChanges,
                        unsubscribeFromChanges,
                        unsubscribeFromHmrEvents
                    }
                }
            });
            currentEntriesHandlingResolve();
            currentEntriesHandlingResolve = undefined;
        }
    }
    await (0, _promises.mkdir)((0, _path.join)(distDir, 'server'), {
        recursive: true
    });
    await (0, _promises.mkdir)((0, _path.join)(distDir, 'static', buildId), {
        recursive: true
    });
    await (0, _promises.writeFile)((0, _path.join)(distDir, 'package.json'), JSON.stringify({
        type: 'commonjs'
    }, null, 2));
    const middlewares = [
        (0, _middlewareturbopack.getOverlayMiddleware)(project),
        (0, _middlewareturbopack.getSourceMapMiddleware)(project)
    ];
    const versionInfoPromise = (0, _hotreloaderwebpack.getVersionInfo)(isTestMode || opts.telemetry.isEnabled);
    let devtoolsFrontendUrl;
    const nodeDebugType = (0, _utils2.getNodeDebugType)();
    if (nodeDebugType) {
        const debugPort = process.debugPort;
        let debugInfo;
        try {
            // It requires to use 127.0.0.1 instead of localhost for server-side fetching.
            const debugInfoList = await fetch(`http://127.0.0.1:${debugPort}/json/list`).then((res)=>res.json());
            debugInfo = debugInfoList[0];
        } catch  {}
        if (debugInfo) {
            devtoolsFrontendUrl = debugInfo.devtoolsFrontendUrl;
        }
    }
    const hotReloader = {
        turbopackProject: project,
        activeWebpackConfigs: undefined,
        serverStats: null,
        edgeServerStats: null,
        async run (req, res, _parsedUrl) {
            var _req_url;
            // intercept page chunks request and ensure them with turbopack
            if ((_req_url = req.url) == null ? void 0 : _req_url.startsWith('/_next/static/chunks/pages/')) {
                const params = (0, _hotreloaderwebpack.matchNextPageBundleRequest)(req.url);
                if (params) {
                    const decodedPagePath = `/${params.path.map((param)=>decodeURIComponent(param)).join('/')}`;
                    const denormalizedPagePath = (0, _denormalizepagepath.denormalizePagePath)(decodedPagePath);
                    await hotReloader.ensurePage({
                        page: denormalizedPagePath,
                        clientOnly: false,
                        definition: undefined,
                        url: req.url
                    }).catch(console.error);
                }
            }
            for (const middleware of middlewares){
                let calledNext = false;
                await middleware(req, res, ()=>{
                    calledNext = true;
                });
                if (!calledNext) {
                    return {
                        finished: true
                    };
                }
            }
            // Request was not finished.
            return {
                finished: undefined
            };
        },
        // TODO: Figure out if socket type can match the NextJsHotReloaderInterface
        onHMR (req, socket, head, onUpgrade) {
            wsServer.handleUpgrade(req, socket, head, (client)=>{
                onUpgrade(client);
                const clientIssues = new Map();
                const subscriptions = new Map();
                clients.add(client);
                clientStates.set(client, {
                    clientIssues,
                    hmrPayloads: new Map(),
                    turbopackUpdates: [],
                    subscriptions
                });
                client.on('close', ()=>{
                    // Remove active subscriptions
                    for (const subscription of subscriptions.values()){
                        subscription.return == null ? void 0 : subscription.return.call(subscription);
                    }
                    clientStates.delete(client);
                    clients.delete(client);
                });
                client.addEventListener('message', ({ data })=>{
                    const parsedData = JSON.parse(typeof data !== 'string' ? data.toString() : data);
                    // Next.js messages
                    switch(parsedData.event){
                        case 'ping':
                            break;
                        case 'span-end':
                            {
                                hotReloaderSpan.manualTraceChild(parsedData.spanName, (0, _turbopackutils.msToNs)(parsedData.startTime), (0, _turbopackutils.msToNs)(parsedData.endTime), parsedData.attributes);
                                break;
                            }
                        case 'client-hmr-latency':
                            hotReloaderSpan.manualTraceChild(parsedData.event, (0, _turbopackutils.msToNs)(parsedData.startTime), (0, _turbopackutils.msToNs)(parsedData.endTime), {
                                updatedModules: parsedData.updatedModules,
                                page: parsedData.page,
                                isPageHidden: parsedData.isPageHidden
                            });
                            break;
                        case 'client-error':
                        case 'client-warning':
                        case 'client-success':
                        case 'server-component-reload-page':
                        case 'client-reload-page':
                        case 'client-removed-page':
                        case 'client-full-reload':
                            const { hadRuntimeError, dependencyChain } = parsedData;
                            if (hadRuntimeError) {
                                _log.warn(_messages.FAST_REFRESH_RUNTIME_RELOAD);
                            }
                            if (Array.isArray(dependencyChain) && typeof dependencyChain[0] === 'string') {
                                const cleanedModulePath = dependencyChain[0].replace(/^\[project\]/, '.').replace(/ \[.*\] \(.*\)$/, '');
                                _log.warn(`Fast Refresh had to perform a full reload when ${cleanedModulePath} changed. Read more: https://nextjs.org/docs/messages/fast-refresh-reload`);
                            }
                            break;
                        case 'client-added-page':
                            break;
                        default:
                            // Might be a Turbopack message...
                            if (!parsedData.type) {
                                throw new Error(`unrecognized HMR message "${data}"`);
                            }
                    }
                    // Turbopack messages
                    switch(parsedData.type){
                        case 'turbopack-subscribe':
                            subscribeToHmrEvents(client, parsedData.path);
                            break;
                        case 'turbopack-unsubscribe':
                            unsubscribeFromHmrEvents(client, parsedData.path);
                            break;
                        default:
                            if (!parsedData.event) {
                                throw new Error(`unrecognized Turbopack HMR message "${data}"`);
                            }
                    }
                });
                const turbopackConnected = {
                    action: _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.TURBOPACK_CONNECTED,
                    data: {
                        sessionId
                    }
                };
                sendToClient(client, turbopackConnected);
                const errors = [];
                for (const entryIssues of currentEntryIssues.values()){
                    for (const issue of entryIssues.values()){
                        if (issue.severity !== 'warning') {
                            errors.push({
                                message: (0, _turbopackutils.formatIssue)(issue)
                            });
                        } else {
                            (0, _turbopackutils.printNonFatalIssue)(issue);
                        }
                    }
                }
                ;
                (async function() {
                    const versionInfo = await versionInfoPromise;
                    const sync = {
                        action: _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.SYNC,
                        errors,
                        warnings: [],
                        hash: '',
                        versionInfo,
                        debug: {
                            devtoolsFrontendUrl
                        }
                    };
                    sendToClient(client, sync);
                })();
            });
        },
        send (action) {
            const payload = JSON.stringify(action);
            for (const client of clients){
                client.send(payload);
            }
        },
        setHmrServerError (_error) {
        // Not implemented yet.
        },
        clearHmrServerError () {
        // Not implemented yet.
        },
        async start () {},
        async stop () {
        // Not implemented yet.
        },
        async getCompilationErrors (page) {
            const appEntryKey = (0, _entrykey.getEntryKey)('app', 'server', page);
            const pagesEntryKey = (0, _entrykey.getEntryKey)('pages', 'server', page);
            const topLevelIssues = currentTopLevelIssues.values();
            const thisEntryIssues = currentEntryIssues.get(appEntryKey) ?? currentEntryIssues.get(pagesEntryKey);
            if (thisEntryIssues !== undefined && thisEntryIssues.size > 0) {
                // If there is an error related to the requesting page we display it instead of the first error
                return [
                    ...topLevelIssues,
                    ...thisEntryIssues.values()
                ].map((issue)=>{
                    const formattedIssue = (0, _turbopackutils.formatIssue)(issue);
                    if (issue.severity === 'warning') {
                        (0, _turbopackutils.printNonFatalIssue)(issue);
                        return null;
                    } else if ((0, _turbopackutils.isWellKnownError)(issue)) {
                        _log.error(formattedIssue);
                    }
                    return new Error(formattedIssue);
                }).filter((error)=>error !== null);
            }
            // Otherwise, return all errors across pages
            const errors = [];
            for (const issue of topLevelIssues){
                if (issue.severity !== 'warning') {
                    errors.push(new Error((0, _turbopackutils.formatIssue)(issue)));
                }
            }
            for (const entryIssues of currentEntryIssues.values()){
                for (const issue of entryIssues.values()){
                    if (issue.severity !== 'warning') {
                        const message = (0, _turbopackutils.formatIssue)(issue);
                        errors.push(new Error(message));
                    } else {
                        (0, _turbopackutils.printNonFatalIssue)(issue);
                    }
                }
            }
            return errors;
        },
        async invalidate ({ // .env files or tsconfig/jsconfig change
        reloadAfterInvalidation }) {
            if (reloadAfterInvalidation) {
                for (const [key, entrypoint] of currentWrittenEntrypoints){
                    clearRequireCache(key, entrypoint, {
                        force: true
                    });
                }
                await (0, _renderserver.clearAllModuleContexts)();
                this.send({
                    action: _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.SERVER_COMPONENT_CHANGES
                });
            }
        },
        async buildFallbackError () {
        // Not implemented yet.
        },
        async ensurePage ({ page: inputPage, // Unused parameters
        // clientOnly,
        appPaths, definition, isApp, url: requestUrl }) {
            return hotReloaderSpan.traceChild('ensure-page', {
                inputPage
            }).traceAsyncFn(async ()=>{
                if (_constants.BLOCKED_PAGES.includes(inputPage) && inputPage !== '/_error') {
                    return;
                }
                await currentEntriesHandling;
                // TODO We shouldn't look into the filesystem again. This should use the information from entrypoints
                let routeDef = definition ?? await (0, _ondemandentryhandler.findPagePathData)(dir, inputPage, nextConfig.pageExtensions, opts.pagesDir, opts.appDir);
                // If the route is actually an app page route, then we should have access
                // to the app route definition, and therefore, the appPaths from it.
                if (!appPaths && definition && (0, _apppageroutedefinition.isAppPageRouteDefinition)(definition)) {
                    appPaths = definition.appPaths;
                }
                let page = routeDef.page;
                if (appPaths) {
                    const normalizedPage = (0, _apppaths.normalizeAppPath)(page);
                    // filter out paths that are not exact matches (e.g. catchall)
                    const matchingAppPaths = appPaths.filter((path)=>(0, _apppaths.normalizeAppPath)(path) === normalizedPage);
                    // the last item in the array is the root page, if there are parallel routes
                    page = matchingAppPaths[matchingAppPaths.length - 1];
                }
                const pathname = (definition == null ? void 0 : definition.pathname) ?? inputPage;
                if (page === '/_error') {
                    let finishBuilding = startBuilding(pathname, requestUrl, false);
                    try {
                        await (0, _turbopackutils.handlePagesErrorRoute)({
                            dev: true,
                            currentEntryIssues,
                            entrypoints: currentEntrypoints,
                            manifestLoader,
                            devRewrites: opts.fsChecker.rewrites,
                            productionRewrites: undefined,
                            logErrors: true,
                            hooks: {
                                subscribeToChanges,
                                handleWrittenEndpoint: (id, result)=>{
                                    clearRequireCache(id, result);
                                    currentWrittenEntrypoints.set(id, result);
                                    assetMapper.setPathsForKey(id, result.clientPaths);
                                }
                            }
                        });
                    } finally{
                        finishBuilding();
                    }
                    return;
                }
                const isInsideAppDir = routeDef.bundlePath.startsWith('app/');
                const isEntryMetadataRouteFile = (0, _ismetadataroute.isMetadataRouteFile)(routeDef.filename.replace(opts.appDir || '', ''), nextConfig.pageExtensions, true);
                const normalizedAppPage = isEntryMetadataRouteFile ? (0, _turbopackutils.normalizedPageToTurbopackStructureRoute)(page, (0, _path.extname)(routeDef.filename)) : page;
                const route = isInsideAppDir ? currentEntrypoints.app.get(normalizedAppPage) : currentEntrypoints.page.get(page);
                if (!route) {
                    // TODO: why is this entry missing in turbopack?
                    if (page === '/middleware') return;
                    if (page === '/src/middleware') return;
                    if (page === '/instrumentation') return;
                    if (page === '/src/instrumentation') return;
                    throw new _utils.PageNotFoundError(`route not found ${page}`);
                }
                // We don't throw on ensureOpts.isApp === true for page-api
                // since this can happen when app pages make
                // api requests to page API routes.
                if (isApp && route.type === 'page') {
                    throw new Error(`mis-matched route type: isApp && page for ${page}`);
                }
                const finishBuilding = startBuilding(pathname, requestUrl, false);
                try {
                    await (0, _turbopackutils.handleRouteType)({
                        dev,
                        page,
                        pathname,
                        route,
                        currentEntryIssues,
                        entrypoints: currentEntrypoints,
                        manifestLoader,
                        readyIds,
                        devRewrites: opts.fsChecker.rewrites,
                        productionRewrites: undefined,
                        logErrors: true,
                        hooks: {
                            subscribeToChanges,
                            handleWrittenEndpoint: (id, result)=>{
                                currentWrittenEntrypoints.set(id, result);
                                clearRequireCache(id, result);
                                assetMapper.setPathsForKey(id, result.clientPaths);
                            }
                        }
                    });
                } finally{
                    finishBuilding();
                }
            });
        }
    };
    handleEntrypointsSubscription().catch((err)=>{
        console.error(err);
        process.exit(1);
    });
    // Write empty manifests
    await currentEntriesHandling;
    await manifestLoader.writeManifests({
        devRewrites: opts.fsChecker.rewrites,
        productionRewrites: undefined,
        entrypoints: currentEntrypoints
    });
    async function handleProjectUpdates() {
        for await (const updateMessage of project.updateInfoSubscribe(30)){
            switch(updateMessage.updateType){
                case 'start':
                    {
                        hotReloader.send({
                            action: _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.BUILDING
                        });
                        break;
                    }
                case 'end':
                    {
                        sendEnqueuedMessages();
                        function addErrors(errorsMap, issues) {
                            for (const issueMap of issues.values()){
                                for (const [key, issue] of issueMap){
                                    if (issue.severity === 'warning') continue;
                                    if (errorsMap.has(key)) continue;
                                    const message = (0, _turbopackutils.formatIssue)(issue);
                                    errorsMap.set(key, {
                                        message,
                                        details: issue.detail ? (0, _turbopackutils.renderStyledStringToErrorAnsi)(issue.detail) : undefined
                                    });
                                }
                            }
                        }
                        const errors = new Map();
                        addErrors(errors, currentEntryIssues);
                        for (const client of clients){
                            const state = clientStates.get(client);
                            if (!state) {
                                continue;
                            }
                            const clientErrors = new Map(errors);
                            addErrors(clientErrors, state.clientIssues);
                            sendToClient(client, {
                                action: _hotreloadertypes.HMR_ACTIONS_SENT_TO_BROWSER.BUILT,
                                hash: String(++hmrHash),
                                errors: [
                                    ...clientErrors.values()
                                ],
                                warnings: []
                            });
                        }
                        if (hmrEventHappened) {
                            const time = updateMessage.value.duration;
                            const timeMessage = time > 2000 ? `${Math.round(time / 100) / 10}s` : `${time}ms`;
                            _log.event(`Compiled in ${timeMessage}`);
                            hmrEventHappened = false;
                        }
                        break;
                    }
                default:
            }
        }
    }
    handleProjectUpdates().catch((err)=>{
        console.error(err);
        process.exit(1);
    });
    return hotReloader;
}

//# sourceMappingURL=hot-reloader-turbopack.js.map