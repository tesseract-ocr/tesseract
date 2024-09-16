import "./require-hook";
import "./node-polyfill-crypto";
import * as log from "../build/output/log";
import loadConfig from "./config";
import path, { resolve } from "path";
import { NON_STANDARD_NODE_ENV } from "../lib/constants";
import { PHASE_DEVELOPMENT_SERVER, SERVER_FILES_MANIFEST } from "../shared/lib/constants";
import { PHASE_PRODUCTION_SERVER } from "../shared/lib/constants";
import { getTracer } from "./lib/trace/tracer";
import { NextServerSpan } from "./lib/trace/constants";
import { formatUrl } from "../shared/lib/router/utils/format-url";
import { checkNodeDebugType } from "./lib/utils";
let ServerImpl;
const getServerImpl = async ()=>{
    if (ServerImpl === undefined) {
        ServerImpl = (await Promise.resolve(require("./next-server"))).default;
    }
    return ServerImpl;
};
const SYMBOL_LOAD_CONFIG = Symbol("next.load_config");
export class NextServer {
    constructor(options){
        this.options = options;
    }
    get hostname() {
        return this.options.hostname;
    }
    get port() {
        return this.options.port;
    }
    getRequestHandler() {
        return async (req, res, parsedUrl)=>{
            return getTracer().trace(NextServerSpan.getRequestHandler, async ()=>{
                const requestHandler = await this.getServerRequestHandler();
                return requestHandler(req, res, parsedUrl);
            });
        };
    }
    getUpgradeHandler() {
        return async (req, socket, head)=>{
            const server = await this.getServer();
            // @ts-expect-error we mark this as protected so it
            // causes an error here
            return server.handleUpgrade.apply(server, [
                req,
                socket,
                head
            ]);
        };
    }
    setAssetPrefix(assetPrefix) {
        if (this.server) {
            this.server.setAssetPrefix(assetPrefix);
        } else {
            this.preparedAssetPrefix = assetPrefix;
        }
    }
    logError(...args) {
        if (this.server) {
            this.server.logError(...args);
        }
    }
    async render(...args) {
        const server = await this.getServer();
        return server.render(...args);
    }
    async renderToHTML(...args) {
        const server = await this.getServer();
        return server.renderToHTML(...args);
    }
    async renderError(...args) {
        const server = await this.getServer();
        return server.renderError(...args);
    }
    async renderErrorToHTML(...args) {
        const server = await this.getServer();
        return server.renderErrorToHTML(...args);
    }
    async render404(...args) {
        const server = await this.getServer();
        return server.render404(...args);
    }
    async prepare(serverFields) {
        if (this.standaloneMode) return;
        const server = await this.getServer();
        if (serverFields) {
            Object.assign(server, serverFields);
        }
        // We shouldn't prepare the server in production,
        // because this code won't be executed when deployed
        if (this.options.dev) {
            await server.prepare();
        }
    }
    async close() {
        const server = await this.getServer();
        return server.close();
    }
    async createServer(options) {
        let ServerImplementation;
        if (options.dev) {
            ServerImplementation = require("./dev/next-dev-server").default;
        } else {
            ServerImplementation = await getServerImpl();
        }
        const server = new ServerImplementation(options);
        return server;
    }
    async [SYMBOL_LOAD_CONFIG]() {
        const dir = resolve(this.options.dir || ".");
        const config = this.options.preloadedConfig || await loadConfig(this.options.dev ? PHASE_DEVELOPMENT_SERVER : PHASE_PRODUCTION_SERVER, dir, {
            customConfig: this.options.conf,
            silent: true
        });
        // check serialized build config when available
        if (process.env.NODE_ENV === "production") {
            try {
                const serializedConfig = require(path.join(dir, ".next", SERVER_FILES_MANIFEST)).config;
                // @ts-expect-error internal field
                config.experimental.isExperimentalCompile = serializedConfig.experimental.isExperimentalCompile;
            } catch (_) {
            // if distDir is customized we don't know until we
            // load the config so fallback to loading the config
            // from next.config.js
            }
        }
        return config;
    }
    async getServer() {
        if (!this.serverPromise) {
            this.serverPromise = this[SYMBOL_LOAD_CONFIG]().then(async (conf)=>{
                if (this.standaloneMode) {
                    process.env.__NEXT_PRIVATE_STANDALONE_CONFIG = JSON.stringify(conf);
                }
                if (!this.options.dev) {
                    if (conf.output === "standalone") {
                        if (!process.env.__NEXT_PRIVATE_STANDALONE_CONFIG) {
                            log.warn(`"next start" does not work with "output: standalone" configuration. Use "node .next/standalone/server.js" instead.`);
                        }
                    } else if (conf.output === "export") {
                        throw new Error(`"next start" does not work with "output: export" configuration. Use "npx serve@latest out" instead.`);
                    }
                }
                this.server = await this.createServer({
                    ...this.options,
                    conf
                });
                if (this.preparedAssetPrefix) {
                    this.server.setAssetPrefix(this.preparedAssetPrefix);
                }
                return this.server;
            });
        }
        return this.serverPromise;
    }
    async getServerRequestHandler() {
        if (this.reqHandler) return this.reqHandler;
        // Memoize request handler creation
        if (!this.reqHandlerPromise) {
            this.reqHandlerPromise = this.getServer().then((server)=>{
                this.reqHandler = getTracer().wrap(NextServerSpan.getServerRequestHandler, server.getRequestHandler().bind(server));
                delete this.reqHandlerPromise;
                return this.reqHandler;
            });
        }
        return this.reqHandlerPromise;
    }
}
class NextCustomServer extends NextServer {
    async prepare() {
        const { getRequestHandlers } = require("./lib/start-server");
        const isNodeDebugging = !!checkNodeDebugType();
        const initResult = await getRequestHandlers({
            dir: this.options.dir,
            port: this.options.port || 3000,
            isDev: !!this.options.dev,
            hostname: this.options.hostname || "localhost",
            minimalMode: this.options.minimalMode,
            isNodeDebugging: !!isNodeDebugging
        });
        this.requestHandler = initResult[0];
        this.upgradeHandler = initResult[1];
        this.renderServer = initResult[2];
    }
    setupWebSocketHandler(customServer, _req) {
        if (!this.didWebSocketSetup) {
            var _req_socket;
            this.didWebSocketSetup = true;
            customServer = customServer || (_req == null ? void 0 : (_req_socket = _req.socket) == null ? void 0 : _req_socket.server);
            if (customServer) {
                customServer.on("upgrade", async (req, socket, head)=>{
                    this.upgradeHandler(req, socket, head);
                });
            }
        }
    }
    getRequestHandler() {
        return async (req, res, parsedUrl)=>{
            this.setupWebSocketHandler(this.options.httpServer, req);
            if (parsedUrl) {
                req.url = formatUrl(parsedUrl);
            }
            return this.requestHandler(req, res);
        };
    }
    async render(...args) {
        let [req, res, pathname, query, parsedUrl] = args;
        this.setupWebSocketHandler(this.options.httpServer, req);
        if (!pathname.startsWith("/")) {
            console.error(`Cannot render page with path "${pathname}"`);
            pathname = `/${pathname}`;
        }
        pathname = pathname === "/index" ? "/" : pathname;
        req.url = formatUrl({
            ...parsedUrl,
            pathname,
            query
        });
        await this.requestHandler(req, res);
        return;
    }
    setAssetPrefix(assetPrefix) {
        super.setAssetPrefix(assetPrefix);
        this.renderServer.setAssetPrefix(assetPrefix);
    }
    constructor(...args){
        super(...args);
        this.standaloneMode = true;
        this.didWebSocketSetup = false;
    }
}
// This file is used for when users run `require('next')`
function createServer(options) {
    // The package is used as a TypeScript plugin.
    if (options && "typescript" in options && "version" in options.typescript) {
        return require("./next-typescript").createTSPlugin(options);
    }
    if (options == null) {
        throw new Error("The server has not been instantiated properly. https://nextjs.org/docs/messages/invalid-server-options");
    }
    if (!("isNextDevCommand" in options) && process.env.NODE_ENV && ![
        "production",
        "development",
        "test"
    ].includes(process.env.NODE_ENV)) {
        log.warn(NON_STANDARD_NODE_ENV);
    }
    if (options.dev && typeof options.dev !== "boolean") {
        console.warn("Warning: 'dev' is not a boolean which could introduce unexpected behavior. https://nextjs.org/docs/messages/invalid-server-options");
    }
    // When the caller is a custom server (using next()).
    if (options.customServer !== false) {
        const dir = resolve(options.dir || ".");
        return new NextCustomServer({
            ...options,
            dir
        });
    }
    // When the caller is Next.js internals (i.e. render worker, start server, etc)
    return new NextServer(options);
}
// Support commonjs `require('next')`
module.exports = createServer;
// exports = module.exports
// Support `import next from 'next'`
export default createServer;

//# sourceMappingURL=next.js.map