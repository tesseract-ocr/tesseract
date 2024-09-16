import loadJsConfig from "../../build/load-jsconfig";
import { decodeMagicIdentifier, MAGIC_IDENTIFIER_REGEX } from "../../shared/lib/magic-identifier";
import { bold, green, magenta, red } from "../../lib/picocolors";
import { HMR_ACTIONS_SENT_TO_BROWSER } from "./hot-reloader-types";
import * as Log from "../../build/output/log";
import { getEntryKey, splitEntryKey } from "./turbopack/entry-key";
export async function getTurbopackJsConfig(dir, nextConfig) {
    const { jsConfig } = await loadJsConfig(dir, nextConfig);
    return jsConfig ?? {
        compilerOptions: {}
    };
}
class ModuleBuildError extends Error {
}
/**
 * Thin stopgap workaround layer to mimic existing wellknown-errors-plugin in webpack's build
 * to emit certain type of errors into cli.
 */ export function isWellKnownError(issue) {
    const { title } = issue;
    const formattedTitle = renderStyledStringToErrorAnsi(title);
    // TODO: add more well known errors
    if (formattedTitle.includes("Module not found") || formattedTitle.includes("Unknown module type")) {
        return true;
    }
    return false;
}
/// Print out an issue to the console which should not block
/// the build by throwing out or blocking error overlay.
export function printNonFatalIssue(issue) {
    if (isRelevantWarning(issue)) {
        Log.warn(formatIssue(issue));
    }
}
function isNodeModulesIssue(issue) {
    return issue.severity === "warning" && issue.filePath.match(/^(?:.*[\\/])?node_modules(?:[\\/].*)?$/) !== null;
}
export function isRelevantWarning(issue) {
    return issue.severity === "warning" && !isNodeModulesIssue(issue);
}
export function formatIssue(issue) {
    const { filePath, title, description, source } = issue;
    let { documentationLink } = issue;
    let formattedTitle = renderStyledStringToErrorAnsi(title).replace(/\n/g, "\n    ");
    // TODO: Use error codes to identify these
    // TODO: Generalize adapting Turbopack errors to Next.js errors
    if (formattedTitle.includes("Module not found")) {
        // For compatiblity with webpack
        // TODO: include columns in webpack errors.
        documentationLink = "https://nextjs.org/docs/messages/module-not-found";
    }
    let formattedFilePath = filePath.replace("[project]/", "./").replaceAll("/./", "/").replace("\\\\?\\", "");
    let message = "";
    if (source && source.range) {
        const { start } = source.range;
        message = `${formattedFilePath}:${start.line + 1}:${start.column + 1}\n${formattedTitle}`;
    } else if (formattedFilePath) {
        message = `${formattedFilePath}\n${formattedTitle}`;
    } else {
        message = formattedTitle;
    }
    message += "\n";
    if ((source == null ? void 0 : source.range) && source.source.content) {
        const { start, end } = source.range;
        const { codeFrameColumns } = require("next/dist/compiled/babel/code-frame");
        message += codeFrameColumns(source.source.content, {
            start: {
                line: start.line + 1,
                column: start.column + 1
            },
            end: {
                line: end.line + 1,
                column: end.column + 1
            }
        }, {
            forceColor: true
        }).trim() + "\n\n";
    }
    if (description) {
        message += renderStyledStringToErrorAnsi(description) + "\n\n";
    }
    // TODO: make it possible to enable this for debugging, but not in tests.
    // if (detail) {
    //   message += renderStyledStringToErrorAnsi(detail) + '\n\n'
    // }
    // TODO: Include a trace from the issue.
    if (documentationLink) {
        message += documentationLink + "\n\n";
    }
    return message;
}
function getIssueKey(issue) {
    return `${issue.severity}-${issue.filePath}-${JSON.stringify(issue.title)}-${JSON.stringify(issue.description)}`;
}
export function processTopLevelIssues(currentTopLevelIssues, result) {
    currentTopLevelIssues.clear();
    for (const issue of result.issues){
        const issueKey = getIssueKey(issue);
        currentTopLevelIssues.set(issueKey, issue);
    }
}
export function processIssues(currentEntryIssues, key, result, throwIssue, logErrors) {
    const newIssues = new Map();
    currentEntryIssues.set(key, newIssues);
    const relevantIssues = new Set();
    for (const issue of result.issues){
        if (issue.severity !== "error" && issue.severity !== "fatal" && issue.severity !== "warning") continue;
        const issueKey = getIssueKey(issue);
        const formatted = formatIssue(issue);
        newIssues.set(issueKey, issue);
        if (issue.severity !== "warning") {
            relevantIssues.add(formatted);
            if (logErrors && isWellKnownError(issue)) {
                Log.error(formatted);
            }
        }
    }
    if (relevantIssues.size && throwIssue) {
        throw new ModuleBuildError([
            ...relevantIssues
        ].join("\n\n"));
    }
}
export function renderStyledStringToErrorAnsi(string) {
    function decodeMagicIdentifiers(str) {
        return str.replaceAll(MAGIC_IDENTIFIER_REGEX, (ident)=>{
            try {
                return magenta(`{${decodeMagicIdentifier(ident)}}`);
            } catch (e) {
                return magenta(`{${ident} (decoding failed: ${e})}`);
            }
        });
    }
    switch(string.type){
        case "text":
            return decodeMagicIdentifiers(string.value);
        case "strong":
            return bold(red(decodeMagicIdentifiers(string.value)));
        case "code":
            return green(decodeMagicIdentifiers(string.value));
        case "line":
            return string.value.map(renderStyledStringToErrorAnsi).join("");
        case "stack":
            return string.value.map(renderStyledStringToErrorAnsi).join("\n");
        default:
            throw new Error("Unknown StyledString type", string);
    }
}
const MILLISECONDS_IN_NANOSECOND = BigInt(1000000);
export function msToNs(ms) {
    return BigInt(Math.floor(ms)) * MILLISECONDS_IN_NANOSECOND;
}
export async function handleRouteType({ dev, page, pathname, route, currentEntryIssues, entrypoints, manifestLoader, readyIds, rewrites, hooks, logErrors }) {
    switch(route.type){
        case "page":
            {
                const clientKey = getEntryKey("pages", "client", page);
                const serverKey = getEntryKey("pages", "server", page);
                try {
                    if (entrypoints.global.app) {
                        const key = getEntryKey("pages", "server", "_app");
                        const writtenEndpoint = await entrypoints.global.app.writeToDisk();
                        hooks == null ? void 0 : hooks.handleWrittenEndpoint(key, writtenEndpoint);
                        processIssues(currentEntryIssues, key, writtenEndpoint, false, logErrors);
                    }
                    await manifestLoader.loadBuildManifest("_app");
                    await manifestLoader.loadPagesManifest("_app");
                    if (entrypoints.global.document) {
                        const key = getEntryKey("pages", "server", "_document");
                        const writtenEndpoint = await entrypoints.global.document.writeToDisk();
                        hooks == null ? void 0 : hooks.handleWrittenEndpoint(key, writtenEndpoint);
                        processIssues(currentEntryIssues, key, writtenEndpoint, false, logErrors);
                    }
                    await manifestLoader.loadPagesManifest("_document");
                    const writtenEndpoint = await route.htmlEndpoint.writeToDisk();
                    hooks == null ? void 0 : hooks.handleWrittenEndpoint(serverKey, writtenEndpoint);
                    const type = writtenEndpoint == null ? void 0 : writtenEndpoint.type;
                    await manifestLoader.loadBuildManifest(page);
                    await manifestLoader.loadPagesManifest(page);
                    if (type === "edge") {
                        await manifestLoader.loadMiddlewareManifest(page, "pages");
                    } else {
                        manifestLoader.deleteMiddlewareManifest(serverKey);
                    }
                    await manifestLoader.loadFontManifest("/_app", "pages");
                    await manifestLoader.loadFontManifest(page, "pages");
                    await manifestLoader.loadLoadableManifest(page, "pages");
                    await manifestLoader.writeManifests({
                        rewrites,
                        pageEntrypoints: entrypoints.page
                    });
                    processIssues(currentEntryIssues, serverKey, writtenEndpoint, false, logErrors);
                } finally{
                    // TODO subscriptions should only be caused by the WebSocket connections
                    // otherwise we don't known when to unsubscribe and this leaking
                    hooks == null ? void 0 : hooks.subscribeToChanges(serverKey, false, route.dataEndpoint, ()=>{
                        // Report the next compilation again
                        readyIds == null ? void 0 : readyIds.delete(pathname);
                        return {
                            event: HMR_ACTIONS_SENT_TO_BROWSER.SERVER_ONLY_CHANGES,
                            pages: [
                                page
                            ]
                        };
                    });
                    hooks == null ? void 0 : hooks.subscribeToChanges(clientKey, false, route.htmlEndpoint, ()=>{
                        return {
                            event: HMR_ACTIONS_SENT_TO_BROWSER.CLIENT_CHANGES
                        };
                    });
                    if (entrypoints.global.document) {
                        hooks == null ? void 0 : hooks.subscribeToChanges(getEntryKey("pages", "server", "_document"), false, entrypoints.global.document, ()=>{
                            return {
                                action: HMR_ACTIONS_SENT_TO_BROWSER.RELOAD_PAGE
                            };
                        });
                    }
                }
                break;
            }
        case "page-api":
            {
                const key = getEntryKey("pages", "server", page);
                const writtenEndpoint = await route.endpoint.writeToDisk();
                hooks == null ? void 0 : hooks.handleWrittenEndpoint(key, writtenEndpoint);
                const type = writtenEndpoint == null ? void 0 : writtenEndpoint.type;
                await manifestLoader.loadPagesManifest(page);
                if (type === "edge") {
                    await manifestLoader.loadMiddlewareManifest(page, "pages");
                } else {
                    manifestLoader.deleteMiddlewareManifest(key);
                }
                await manifestLoader.loadLoadableManifest(page, "pages");
                await manifestLoader.writeManifests({
                    rewrites,
                    pageEntrypoints: entrypoints.page
                });
                processIssues(currentEntryIssues, key, writtenEndpoint, true, logErrors);
                break;
            }
        case "app-page":
            {
                const key = getEntryKey("app", "server", page);
                const writtenEndpoint = await route.htmlEndpoint.writeToDisk();
                hooks == null ? void 0 : hooks.handleWrittenEndpoint(key, writtenEndpoint);
                // TODO subscriptions should only be caused by the WebSocket connections
                // otherwise we don't known when to unsubscribe and this leaking
                hooks == null ? void 0 : hooks.subscribeToChanges(key, true, route.rscEndpoint, (change)=>{
                    if (change.issues.some((issue)=>issue.severity === "error")) {
                        // Ignore any updates that has errors
                        // There will be another update without errors eventually
                        return;
                    }
                    // Report the next compilation again
                    readyIds == null ? void 0 : readyIds.delete(pathname);
                    return {
                        action: HMR_ACTIONS_SENT_TO_BROWSER.SERVER_COMPONENT_CHANGES
                    };
                });
                const type = writtenEndpoint == null ? void 0 : writtenEndpoint.type;
                if (type === "edge") {
                    await manifestLoader.loadMiddlewareManifest(page, "app");
                } else {
                    manifestLoader.deleteMiddlewareManifest(key);
                }
                await manifestLoader.loadAppBuildManifest(page);
                await manifestLoader.loadBuildManifest(page, "app");
                await manifestLoader.loadAppPathsManifest(page);
                await manifestLoader.loadActionManifest(page);
                await manifestLoader.loadLoadableManifest(page, "app");
                await manifestLoader.loadFontManifest(page, "app");
                await manifestLoader.writeManifests({
                    rewrites,
                    pageEntrypoints: entrypoints.page
                });
                processIssues(currentEntryIssues, key, writtenEndpoint, dev, logErrors);
                break;
            }
        case "app-route":
            {
                const key = getEntryKey("app", "server", page);
                const writtenEndpoint = await route.endpoint.writeToDisk();
                hooks == null ? void 0 : hooks.handleWrittenEndpoint(key, writtenEndpoint);
                const type = writtenEndpoint == null ? void 0 : writtenEndpoint.type;
                await manifestLoader.loadAppPathsManifest(page);
                if (type === "edge") {
                    await manifestLoader.loadMiddlewareManifest(page, "app");
                } else {
                    manifestLoader.deleteMiddlewareManifest(key);
                }
                await manifestLoader.writeManifests({
                    rewrites,
                    pageEntrypoints: entrypoints.page
                });
                processIssues(currentEntryIssues, key, writtenEndpoint, true, logErrors);
                break;
            }
        default:
            {
                throw new Error(`unknown route type ${route.type} for ${page}`);
            }
    }
}
/**
 * Maintains a mapping between entrypoins and the corresponding client asset paths.
 */ export class AssetMapper {
    /**
   * Overrides asset paths for a key and updates the mapping from path to key.
   *
   * @param key
   * @param assetPaths asset paths relative to the .next directory
   */ setPathsForKey(key, assetPaths) {
        this.delete(key);
        const newAssetPaths = new Set(assetPaths);
        this.entryMap.set(key, newAssetPaths);
        for (const assetPath of newAssetPaths){
            let assetPathKeys = this.assetMap.get(assetPath);
            if (!assetPathKeys) {
                assetPathKeys = new Set();
                this.assetMap.set(assetPath, assetPathKeys);
            }
            assetPathKeys.add(key);
        }
    }
    /**
   * Deletes the key and any asset only referenced by this key.
   *
   * @param key
   */ delete(key) {
        for (const assetPath of this.getAssetPathsByKey(key)){
            const assetPathKeys = this.assetMap.get(assetPath);
            assetPathKeys == null ? void 0 : assetPathKeys.delete(key);
            if (!(assetPathKeys == null ? void 0 : assetPathKeys.size)) {
                this.assetMap.delete(assetPath);
            }
        }
        this.entryMap.delete(key);
    }
    getAssetPathsByKey(key) {
        return Array.from(this.entryMap.get(key) ?? []);
    }
    getKeysByAsset(path) {
        return Array.from(this.assetMap.get(path) ?? []);
    }
    keys() {
        return this.entryMap.keys();
    }
    constructor(){
        this.entryMap = new Map();
        this.assetMap = new Map();
    }
}
export function hasEntrypointForKey(entrypoints, key, assetMapper) {
    const { type, page } = splitEntryKey(key);
    switch(type){
        case "app":
            return entrypoints.app.has(page);
        case "pages":
            switch(page){
                case "_app":
                    return entrypoints.global.app != null;
                case "_document":
                    return entrypoints.global.document != null;
                case "_error":
                    return entrypoints.global.error != null;
                default:
                    return entrypoints.page.has(page);
            }
        case "root":
            switch(page){
                case "middleware":
                    return entrypoints.global.middleware != null;
                case "instrumentation":
                    return entrypoints.global.instrumentation != null;
                default:
                    return false;
            }
        case "assets":
            if (!assetMapper) {
                return false;
            }
            return assetMapper.getKeysByAsset(page).some((pageKey)=>hasEntrypointForKey(entrypoints, pageKey, assetMapper));
        default:
            {
                // validation that we covered all cases, this should never run.
                // eslint-disable-next-line @typescript-eslint/no-unused-vars
                const _ = type;
                return false;
            }
    }
}
export async function handleEntrypoints({ entrypoints, currentEntrypoints, currentEntryIssues, manifestLoader, nextConfig, rewrites, logErrors, dev }) {
    currentEntrypoints.global.app = entrypoints.pagesAppEndpoint;
    currentEntrypoints.global.document = entrypoints.pagesDocumentEndpoint;
    currentEntrypoints.global.error = entrypoints.pagesErrorEndpoint;
    currentEntrypoints.global.instrumentation = entrypoints.instrumentation;
    currentEntrypoints.page.clear();
    currentEntrypoints.app.clear();
    for (const [pathname, route] of entrypoints.routes){
        switch(route.type){
            case "page":
            case "page-api":
                currentEntrypoints.page.set(pathname, route);
                break;
            case "app-page":
                {
                    route.pages.forEach((page)=>{
                        currentEntrypoints.app.set(page.originalName, {
                            type: "app-page",
                            ...page
                        });
                    });
                    break;
                }
            case "app-route":
                {
                    currentEntrypoints.app.set(route.originalName, route);
                    break;
                }
            default:
                Log.info(`skipping ${pathname} (${route.type})`);
                break;
        }
    }
    if (dev) {
        await handleEntrypointsDevCleanup({
            currentEntryIssues,
            currentEntrypoints,
            ...dev
        });
    }
    const { middleware, instrumentation } = entrypoints;
    // We check for explicit true/false, since it's initialized to
    // undefined during the first loop (middlewareChanges event is
    // unnecessary during the first serve)
    if (currentEntrypoints.global.middleware && !middleware) {
        const key = getEntryKey("root", "server", "middleware");
        // Went from middleware to no middleware
        await (dev == null ? void 0 : dev.hooks.unsubscribeFromChanges(key));
        currentEntryIssues.delete(key);
        dev == null ? void 0 : dev.hooks.sendHmr("middleware", {
            event: HMR_ACTIONS_SENT_TO_BROWSER.MIDDLEWARE_CHANGES
        });
    } else if (!currentEntrypoints.global.middleware && middleware) {
        // Went from no middleware to middleware
        dev == null ? void 0 : dev.hooks.sendHmr("middleware", {
            event: HMR_ACTIONS_SENT_TO_BROWSER.MIDDLEWARE_CHANGES
        });
    }
    currentEntrypoints.global.middleware = middleware;
    if (nextConfig.experimental.instrumentationHook && instrumentation) {
        const processInstrumentation = async (name, prop)=>{
            const key = getEntryKey("root", "server", name);
            const writtenEndpoint = await instrumentation[prop].writeToDisk();
            dev == null ? void 0 : dev.hooks.handleWrittenEndpoint(key, writtenEndpoint);
            processIssues(currentEntryIssues, key, writtenEndpoint, false, logErrors);
        };
        await processInstrumentation("instrumentation.nodeJs", "nodeJs");
        await processInstrumentation("instrumentation.edge", "edge");
        await manifestLoader.loadMiddlewareManifest("instrumentation", "instrumentation");
        await manifestLoader.writeManifests({
            rewrites: rewrites,
            pageEntrypoints: currentEntrypoints.page
        });
        if (dev) {
            dev.serverFields.actualInstrumentationHookFile = "/instrumentation";
            await dev.hooks.propagateServerField("actualInstrumentationHookFile", dev.serverFields.actualInstrumentationHookFile);
        }
    } else {
        if (dev) {
            dev.serverFields.actualInstrumentationHookFile = undefined;
            await dev.hooks.propagateServerField("actualInstrumentationHookFile", dev.serverFields.actualInstrumentationHookFile);
        }
    }
    if (middleware) {
        const key = getEntryKey("root", "server", "middleware");
        const endpoint = middleware.endpoint;
        async function processMiddleware() {
            const writtenEndpoint = await endpoint.writeToDisk();
            dev == null ? void 0 : dev.hooks.handleWrittenEndpoint(key, writtenEndpoint);
            processIssues(currentEntryIssues, key, writtenEndpoint, false, logErrors);
            await manifestLoader.loadMiddlewareManifest("middleware", "middleware");
            if (dev) {
                var _manifestLoader_getMiddlewareManifest;
                dev.serverFields.middleware = {
                    match: null,
                    page: "/",
                    matchers: (_manifestLoader_getMiddlewareManifest = manifestLoader.getMiddlewareManifest(key)) == null ? void 0 : _manifestLoader_getMiddlewareManifest.middleware["/"].matchers
                };
            }
        }
        await processMiddleware();
        dev == null ? void 0 : dev.hooks.subscribeToChanges(key, false, endpoint, async ()=>{
            const finishBuilding = dev.hooks.startBuilding("middleware", undefined, true);
            await processMiddleware();
            await dev.hooks.propagateServerField("actualMiddlewareFile", dev.serverFields.actualMiddlewareFile);
            await dev.hooks.propagateServerField("middleware", dev.serverFields.middleware);
            await manifestLoader.writeManifests({
                rewrites: rewrites,
                pageEntrypoints: currentEntrypoints.page
            });
            finishBuilding == null ? void 0 : finishBuilding();
            return {
                event: HMR_ACTIONS_SENT_TO_BROWSER.MIDDLEWARE_CHANGES
            };
        });
    } else {
        manifestLoader.deleteMiddlewareManifest(getEntryKey("root", "server", "middleware"));
        if (dev) {
            dev.serverFields.actualMiddlewareFile = undefined;
            dev.serverFields.middleware = undefined;
        }
    }
    if (dev) {
        await dev.hooks.propagateServerField("actualMiddlewareFile", dev.serverFields.actualMiddlewareFile);
        await dev.hooks.propagateServerField("middleware", dev.serverFields.middleware);
    }
}
async function handleEntrypointsDevCleanup({ currentEntryIssues, currentEntrypoints, assetMapper, changeSubscriptions, clients, clientStates, hooks }) {
    // this needs to be first as `hasEntrypointForKey` uses the `assetMapper`
    for (const key of assetMapper.keys()){
        if (!hasEntrypointForKey(currentEntrypoints, key, assetMapper)) {
            assetMapper.delete(key);
        }
    }
    for (const key of changeSubscriptions.keys()){
        // middleware is handled separately
        if (!hasEntrypointForKey(currentEntrypoints, key, assetMapper)) {
            await hooks.unsubscribeFromChanges(key);
        }
    }
    for (const [key] of currentEntryIssues){
        if (!hasEntrypointForKey(currentEntrypoints, key, assetMapper)) {
            currentEntryIssues.delete(key);
        }
    }
    for (const client of clients){
        const state = clientStates.get(client);
        if (!state) {
            continue;
        }
        for (const key of state.clientIssues.keys()){
            if (!hasEntrypointForKey(currentEntrypoints, key, assetMapper)) {
                state.clientIssues.delete(key);
            }
        }
        for (const id of state.subscriptions.keys()){
            if (!hasEntrypointForKey(currentEntrypoints, getEntryKey("assets", "client", id), assetMapper)) {
                hooks.unsubscribeFromHmrEvents(client, id);
            }
        }
    }
}
export async function handlePagesErrorRoute({ currentEntryIssues, entrypoints, manifestLoader, rewrites, logErrors, hooks }) {
    if (entrypoints.global.app) {
        const key = getEntryKey("pages", "server", "_app");
        const writtenEndpoint = await entrypoints.global.app.writeToDisk();
        hooks == null ? void 0 : hooks.handleWrittenEndpoint(key, writtenEndpoint);
        hooks == null ? void 0 : hooks.subscribeToChanges(key, false, entrypoints.global.app, ()=>{
            // There's a special case for this in `../client/page-bootstrap.ts`.
            // https://github.com/vercel/next.js/blob/08d7a7e5189a835f5dcb82af026174e587575c0e/packages/next/src/client/page-bootstrap.ts#L69-L71
            return {
                event: HMR_ACTIONS_SENT_TO_BROWSER.CLIENT_CHANGES
            };
        });
        processIssues(currentEntryIssues, key, writtenEndpoint, false, logErrors);
    }
    await manifestLoader.loadBuildManifest("_app");
    await manifestLoader.loadPagesManifest("_app");
    await manifestLoader.loadFontManifest("_app");
    if (entrypoints.global.document) {
        const key = getEntryKey("pages", "server", "_document");
        const writtenEndpoint = await entrypoints.global.document.writeToDisk();
        hooks == null ? void 0 : hooks.handleWrittenEndpoint(key, writtenEndpoint);
        hooks == null ? void 0 : hooks.subscribeToChanges(key, false, entrypoints.global.document, ()=>{
            return {
                action: HMR_ACTIONS_SENT_TO_BROWSER.RELOAD_PAGE
            };
        });
        processIssues(currentEntryIssues, key, writtenEndpoint, false, logErrors);
    }
    await manifestLoader.loadPagesManifest("_document");
    if (entrypoints.global.error) {
        const key = getEntryKey("pages", "server", "_error");
        const writtenEndpoint = await entrypoints.global.error.writeToDisk();
        hooks == null ? void 0 : hooks.handleWrittenEndpoint(key, writtenEndpoint);
        hooks == null ? void 0 : hooks.subscribeToChanges(key, false, entrypoints.global.error, ()=>{
            // There's a special case for this in `../client/page-bootstrap.ts`.
            // https://github.com/vercel/next.js/blob/08d7a7e5189a835f5dcb82af026174e587575c0e/packages/next/src/client/page-bootstrap.ts#L69-L71
            return {
                event: HMR_ACTIONS_SENT_TO_BROWSER.CLIENT_CHANGES
            };
        });
        processIssues(currentEntryIssues, key, writtenEndpoint, false, logErrors);
    }
    await manifestLoader.loadBuildManifest("_error");
    await manifestLoader.loadPagesManifest("_error");
    await manifestLoader.loadFontManifest("_error");
    await manifestLoader.writeManifests({
        rewrites,
        pageEntrypoints: entrypoints.page
    });
}

//# sourceMappingURL=turbopack-utils.js.map