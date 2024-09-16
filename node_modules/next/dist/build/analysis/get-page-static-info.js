"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getMiddlewareMatchers: null,
    getPageStaticInfo: null,
    getRSCModuleInformation: null,
    isDynamicMetadataRoute: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getMiddlewareMatchers: function() {
        return getMiddlewareMatchers;
    },
    getPageStaticInfo: function() {
        return getPageStaticInfo;
    },
    getRSCModuleInformation: function() {
        return getRSCModuleInformation;
    },
    isDynamicMetadataRoute: function() {
        return isDynamicMetadataRoute;
    }
});
const _fs = require("fs");
const _lrucache = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/lru-cache"));
const _picomatch = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/picomatch"));
const _extractconstvalue = require("./extract-const-value");
const _parsemodule = require("./parse-module");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../output/log"));
const _constants = require("../../lib/constants");
const _loadcustomroutes = require("../../lib/load-custom-routes");
const _trytoparsepath = require("../../lib/try-to-parse-path");
const _isapiroute = require("../../lib/is-api-route");
const _isedgeruntime = require("../../lib/is-edge-runtime");
const _constants1 = require("../../shared/lib/constants");
const _pagetypes = require("../../lib/page-types");
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
// TODO: migrate preferredRegion here
// Don't forget to update the next-types-plugin file as well
const AUTHORIZED_EXTRA_ROUTER_PROPS = [
    "maxDuration"
];
const CLIENT_MODULE_LABEL = /\/\* __next_internal_client_entry_do_not_use__ ([^ ]*) (cjs|auto) \*\//;
const ACTION_MODULE_LABEL = /\/\* __next_internal_action_entry_do_not_use__ (\{[^}]+\}) \*\//;
const CLIENT_DIRECTIVE = "use client";
const SERVER_ACTION_DIRECTIVE = "use server";
function getRSCModuleInformation(source, isReactServerLayer) {
    var _clientInfoMatch_;
    const actionsJson = source.match(ACTION_MODULE_LABEL);
    const actions = actionsJson ? Object.values(JSON.parse(actionsJson[1])) : undefined;
    const clientInfoMatch = source.match(CLIENT_MODULE_LABEL);
    const isClientRef = !!clientInfoMatch;
    if (!isReactServerLayer) {
        return {
            type: _constants1.RSC_MODULE_TYPES.client,
            actions,
            isClientRef
        };
    }
    const clientRefs = clientInfoMatch == null ? void 0 : (_clientInfoMatch_ = clientInfoMatch[1]) == null ? void 0 : _clientInfoMatch_.split(",");
    const clientEntryType = clientInfoMatch == null ? void 0 : clientInfoMatch[2];
    const type = clientRefs ? _constants1.RSC_MODULE_TYPES.client : _constants1.RSC_MODULE_TYPES.server;
    return {
        type,
        actions,
        clientRefs,
        clientEntryType,
        isClientRef
    };
}
const warnedInvalidValueMap = {
    runtime: new Map(),
    preferredRegion: new Map()
};
function warnInvalidValue(pageFilePath, key, message) {
    if (warnedInvalidValueMap[key].has(pageFilePath)) return;
    _log.warn(`Next.js can't recognize the exported \`${key}\` field in "${pageFilePath}" as ${message}.` + "\n" + "The default runtime will be used instead.");
    warnedInvalidValueMap[key].set(pageFilePath, true);
}
/**
 * Receives a parsed AST from SWC and checks if it belongs to a module that
 * requires a runtime to be specified. Those are:
 *   - Modules with `export function getStaticProps | getServerSideProps`
 *   - Modules with `export { getStaticProps | getServerSideProps } <from ...>`
 *   - Modules with `export const runtime = ...`
 */ function checkExports(swcAST, pageFilePath) {
    const exportsSet = new Set([
        "getStaticProps",
        "getServerSideProps",
        "generateImageMetadata",
        "generateSitemaps",
        "generateStaticParams"
    ]);
    if (Array.isArray(swcAST == null ? void 0 : swcAST.body)) {
        try {
            let runtime;
            let preferredRegion;
            let ssr = false;
            let ssg = false;
            let generateImageMetadata = false;
            let generateSitemaps = false;
            let generateStaticParams = false;
            let extraProperties = new Set();
            let directives = new Set();
            let hasLeadingNonDirectiveNode = false;
            for (const node of swcAST.body){
                var _node_declaration, _node_declaration1, _node_declaration_identifier, _node_declaration2;
                // There should be no non-string literals nodes before directives
                if (node.type === "ExpressionStatement" && node.expression.type === "StringLiteral") {
                    if (!hasLeadingNonDirectiveNode) {
                        const directive = node.expression.value;
                        if (CLIENT_DIRECTIVE === directive) {
                            directives.add("client");
                        }
                        if (SERVER_ACTION_DIRECTIVE === directive) {
                            directives.add("server");
                        }
                    }
                } else {
                    hasLeadingNonDirectiveNode = true;
                }
                if (node.type === "ExportDeclaration" && ((_node_declaration = node.declaration) == null ? void 0 : _node_declaration.type) === "VariableDeclaration") {
                    var _node_declaration3;
                    for (const declaration of (_node_declaration3 = node.declaration) == null ? void 0 : _node_declaration3.declarations){
                        if (declaration.id.value === "runtime") {
                            runtime = declaration.init.value;
                        } else if (declaration.id.value === "preferredRegion") {
                            if (declaration.init.type === "ArrayExpression") {
                                const elements = [];
                                for (const element of declaration.init.elements){
                                    const { expression } = element;
                                    if (expression.type !== "StringLiteral") {
                                        continue;
                                    }
                                    elements.push(expression.value);
                                }
                                preferredRegion = elements;
                            } else {
                                preferredRegion = declaration.init.value;
                            }
                        } else {
                            extraProperties.add(declaration.id.value);
                        }
                    }
                }
                if (node.type === "ExportDeclaration" && ((_node_declaration1 = node.declaration) == null ? void 0 : _node_declaration1.type) === "FunctionDeclaration" && exportsSet.has((_node_declaration_identifier = node.declaration.identifier) == null ? void 0 : _node_declaration_identifier.value)) {
                    const id = node.declaration.identifier.value;
                    ssg = id === "getStaticProps";
                    ssr = id === "getServerSideProps";
                    generateImageMetadata = id === "generateImageMetadata";
                    generateSitemaps = id === "generateSitemaps";
                    generateStaticParams = id === "generateStaticParams";
                }
                if (node.type === "ExportDeclaration" && ((_node_declaration2 = node.declaration) == null ? void 0 : _node_declaration2.type) === "VariableDeclaration") {
                    var _node_declaration_declarations_, _node_declaration4;
                    const id = (_node_declaration4 = node.declaration) == null ? void 0 : (_node_declaration_declarations_ = _node_declaration4.declarations[0]) == null ? void 0 : _node_declaration_declarations_.id.value;
                    if (exportsSet.has(id)) {
                        ssg = id === "getStaticProps";
                        ssr = id === "getServerSideProps";
                        generateImageMetadata = id === "generateImageMetadata";
                        generateSitemaps = id === "generateSitemaps";
                        generateStaticParams = id === "generateStaticParams";
                    }
                }
                if (node.type === "ExportNamedDeclaration") {
                    const values = node.specifiers.map((specifier)=>{
                        var _specifier_orig, _specifier_orig1;
                        return specifier.type === "ExportSpecifier" && ((_specifier_orig = specifier.orig) == null ? void 0 : _specifier_orig.type) === "Identifier" && ((_specifier_orig1 = specifier.orig) == null ? void 0 : _specifier_orig1.value);
                    });
                    for (const value of values){
                        if (!ssg && value === "getStaticProps") ssg = true;
                        if (!ssr && value === "getServerSideProps") ssr = true;
                        if (!generateImageMetadata && value === "generateImageMetadata") generateImageMetadata = true;
                        if (!generateSitemaps && value === "generateSitemaps") generateSitemaps = true;
                        if (!generateStaticParams && value === "generateStaticParams") generateStaticParams = true;
                        if (!runtime && value === "runtime") warnInvalidValue(pageFilePath, "runtime", "it was not assigned to a string literal");
                        if (!preferredRegion && value === "preferredRegion") warnInvalidValue(pageFilePath, "preferredRegion", "it was not assigned to a string literal or an array of string literals");
                    }
                }
            }
            return {
                ssr,
                ssg,
                runtime,
                preferredRegion,
                generateImageMetadata,
                generateSitemaps,
                generateStaticParams,
                extraProperties,
                directives
            };
        } catch (err) {}
    }
    return {
        ssg: false,
        ssr: false,
        runtime: undefined,
        preferredRegion: undefined,
        generateImageMetadata: false,
        generateSitemaps: false,
        generateStaticParams: false,
        extraProperties: undefined,
        directives: undefined
    };
}
async function tryToReadFile(filePath, shouldThrow) {
    try {
        return await _fs.promises.readFile(filePath, {
            encoding: "utf8"
        });
    } catch (error) {
        if (shouldThrow) {
            error.message = `Next.js ERROR: Failed to read file ${filePath}:\n${error.message}`;
            throw error;
        }
    }
}
function getMiddlewareMatchers(matcherOrMatchers, nextConfig) {
    let matchers = [];
    if (Array.isArray(matcherOrMatchers)) {
        matchers = matcherOrMatchers;
    } else {
        matchers.push(matcherOrMatchers);
    }
    const { i18n } = nextConfig;
    const originalSourceMap = new Map();
    let routes = matchers.map((m)=>{
        let middleware = typeof m === "string" ? {
            source: m
        } : m;
        if (middleware) {
            originalSourceMap.set(middleware, middleware.source);
        }
        return middleware;
    });
    // check before we process the routes and after to ensure
    // they are still valid
    (0, _loadcustomroutes.checkCustomRoutes)(routes, "middleware");
    routes = routes.map((r)=>{
        let { source } = r;
        const isRoot = source === "/";
        if ((i18n == null ? void 0 : i18n.locales) && r.locale !== false) {
            source = `/:nextInternalLocale((?!_next/)[^/.]{1,})${isRoot ? "" : source}`;
        }
        source = `/:nextData(_next/data/[^/]{1,})?${source}${isRoot ? `(${nextConfig.i18n ? "|\\.json|" : ""}/?index|/?index\\.json)?` : "(.json)?"}`;
        if (nextConfig.basePath) {
            source = `${nextConfig.basePath}${source}`;
        }
        r.source = source;
        return r;
    });
    (0, _loadcustomroutes.checkCustomRoutes)(routes, "middleware");
    return routes.map((r)=>{
        const { source, ...rest } = r;
        const parsedPage = (0, _trytoparsepath.tryToParsePath)(source);
        if (parsedPage.error || !parsedPage.regexStr) {
            throw new Error(`Invalid source: ${source}`);
        }
        const originalSource = originalSourceMap.get(r);
        return {
            ...rest,
            regexp: parsedPage.regexStr,
            originalSource: originalSource || source
        };
    });
}
function getMiddlewareConfig(pageFilePath, config, nextConfig) {
    const result = {};
    if (config.matcher) {
        result.matchers = getMiddlewareMatchers(config.matcher, nextConfig);
    }
    if (typeof config.regions === "string" || Array.isArray(config.regions)) {
        result.regions = config.regions;
    } else if (typeof config.regions !== "undefined") {
        _log.warn(`The \`regions\` config was ignored: config must be empty, a string or an array of strings. (${pageFilePath})`);
    }
    if (config.unstable_allowDynamic) {
        result.unstable_allowDynamicGlobs = Array.isArray(config.unstable_allowDynamic) ? config.unstable_allowDynamic : [
            config.unstable_allowDynamic
        ];
        for (const glob of result.unstable_allowDynamicGlobs ?? []){
            try {
                (0, _picomatch.default)(glob);
            } catch (err) {
                throw new Error(`${pageFilePath} exported 'config.unstable_allowDynamic' contains invalid pattern '${glob}': ${err.message}`);
            }
        }
    }
    return result;
}
const apiRouteWarnings = new _lrucache.default({
    max: 250
});
function warnAboutExperimentalEdge(apiRoute) {
    if (process.env.NODE_ENV === "production" && process.env.NEXT_PRIVATE_BUILD_WORKER === "1") {
        return;
    }
    if (apiRouteWarnings.has(apiRoute)) {
        return;
    }
    _log.warn(apiRoute ? `${apiRoute} provided runtime 'experimental-edge'. It can be updated to 'edge' instead.` : `You are using an experimental edge runtime, the API might change.`);
    apiRouteWarnings.set(apiRoute, 1);
}
const warnedUnsupportedValueMap = new _lrucache.default({
    max: 250
});
function warnAboutUnsupportedValue(pageFilePath, page, error) {
    if (warnedUnsupportedValueMap.has(pageFilePath)) {
        return;
    }
    _log.warn(`Next.js can't recognize the exported \`config\` field in ` + (page ? `route "${page}"` : `"${pageFilePath}"`) + ":\n" + error.message + (error.path ? ` at "${error.path}"` : "") + ".\n" + "The default config will be used instead.\n" + "Read More - https://nextjs.org/docs/messages/invalid-page-config");
    warnedUnsupportedValueMap.set(pageFilePath, true);
}
async function isDynamicMetadataRoute(pageFilePath) {
    const fileContent = await tryToReadFile(pageFilePath, true) || "";
    if (/generateImageMetadata|generateSitemaps/.test(fileContent)) {
        const swcAST = await (0, _parsemodule.parseModule)(pageFilePath, fileContent);
        const exportsInfo = checkExports(swcAST, pageFilePath);
        return !!(exportsInfo.generateImageMetadata || exportsInfo.generateSitemaps);
    }
    return false;
}
async function getPageStaticInfo(params) {
    const { isDev, pageFilePath, nextConfig, page, pageType } = params;
    const fileContent = await tryToReadFile(pageFilePath, !isDev) || "";
    if (/(?<!(_jsx|jsx-))runtime|preferredRegion|getStaticProps|getServerSideProps|generateStaticParams|export const/.test(fileContent)) {
        const swcAST = await (0, _parsemodule.parseModule)(pageFilePath, fileContent);
        const { ssg, ssr, runtime, preferredRegion, generateStaticParams, extraProperties, directives } = checkExports(swcAST, pageFilePath);
        const rscInfo = getRSCModuleInformation(fileContent, true);
        const rsc = rscInfo.type;
        // default / failsafe value for config
        let config// TODO: type this as unknown
        ;
        try {
            config = (0, _extractconstvalue.extractExportedConstValue)(swcAST, "config");
        } catch (e) {
            if (e instanceof _extractconstvalue.UnsupportedValueError) {
                warnAboutUnsupportedValue(pageFilePath, page, e);
            }
        // `export config` doesn't exist, or other unknown error thrown by swc, silence them
        }
        const extraConfig = {};
        if (extraProperties && pageType === _pagetypes.PAGE_TYPES.APP) {
            for (const prop of extraProperties){
                if (!AUTHORIZED_EXTRA_ROUTER_PROPS.includes(prop)) continue;
                try {
                    extraConfig[prop] = (0, _extractconstvalue.extractExportedConstValue)(swcAST, prop);
                } catch (e) {
                    if (e instanceof _extractconstvalue.UnsupportedValueError) {
                        warnAboutUnsupportedValue(pageFilePath, page, e);
                    }
                }
            }
        } else if (pageType === _pagetypes.PAGE_TYPES.PAGES) {
            for(const key in config){
                if (!AUTHORIZED_EXTRA_ROUTER_PROPS.includes(key)) continue;
                extraConfig[key] = config[key];
            }
        }
        if (pageType === _pagetypes.PAGE_TYPES.APP) {
            if (config) {
                let message = `Page config in ${pageFilePath} is deprecated. Replace \`export const config=…\` with the following:`;
                if (config.runtime) {
                    message += `\n  - \`export const runtime = ${JSON.stringify(config.runtime)}\``;
                }
                if (config.regions) {
                    message += `\n  - \`export const preferredRegion = ${JSON.stringify(config.regions)}\``;
                }
                message += `\nVisit https://nextjs.org/docs/app/api-reference/file-conventions/route-segment-config for more information.`;
                if (isDev) {
                    _log.warnOnce(message);
                } else {
                    throw new Error(message);
                }
                config = {};
            }
        }
        if (!config) config = {};
        // We use `export const config = { runtime: '...' }` to specify the page runtime for pages/.
        // In the new app directory, we prefer to use `export const runtime = '...'`
        // and deprecate the old way. To prevent breaking changes for `pages`, we use the exported config
        // as the fallback value.
        let resolvedRuntime;
        if (pageType === _pagetypes.PAGE_TYPES.APP) {
            resolvedRuntime = runtime;
        } else {
            resolvedRuntime = runtime || config.runtime;
        }
        if (typeof resolvedRuntime !== "undefined" && resolvedRuntime !== _constants.SERVER_RUNTIME.nodejs && !(0, _isedgeruntime.isEdgeRuntime)(resolvedRuntime)) {
            const options = Object.values(_constants.SERVER_RUNTIME).join(", ");
            const message = typeof resolvedRuntime !== "string" ? `The \`runtime\` config must be a string. Please leave it empty or choose one of: ${options}` : `Provided runtime "${resolvedRuntime}" is not supported. Please leave it empty or choose one of: ${options}`;
            if (isDev) {
                _log.error(message);
            } else {
                throw new Error(message);
            }
        }
        const requiresServerRuntime = ssr || ssg || pageType === _pagetypes.PAGE_TYPES.APP;
        const isAnAPIRoute = (0, _isapiroute.isAPIRoute)(page == null ? void 0 : page.replace(/^(?:\/src)?\/pages\//, "/"));
        resolvedRuntime = (0, _isedgeruntime.isEdgeRuntime)(resolvedRuntime) || requiresServerRuntime ? resolvedRuntime : undefined;
        if (resolvedRuntime === _constants.SERVER_RUNTIME.experimentalEdge) {
            warnAboutExperimentalEdge(isAnAPIRoute ? page : null);
        }
        if (resolvedRuntime === _constants.SERVER_RUNTIME.edge && pageType === _pagetypes.PAGE_TYPES.PAGES && page && !isAnAPIRoute) {
            const message = `Page ${page} provided runtime 'edge', the edge runtime for rendering is currently experimental. Use runtime 'experimental-edge' instead.`;
            if (isDev) {
                _log.error(message);
            } else {
                throw new Error(message);
            }
        }
        const middlewareConfig = getMiddlewareConfig(page ?? "middleware/edge API route", config, nextConfig);
        if (pageType === _pagetypes.PAGE_TYPES.APP && (directives == null ? void 0 : directives.has("client")) && generateStaticParams) {
            throw new Error(`Page "${page}" cannot use both "use client" and export function "generateStaticParams()".`);
        }
        return {
            ssr,
            ssg,
            rsc,
            generateStaticParams,
            amp: config.amp || false,
            ...middlewareConfig && {
                middleware: middlewareConfig
            },
            ...resolvedRuntime && {
                runtime: resolvedRuntime
            },
            preferredRegion,
            extraConfig
        };
    }
    return {
        ssr: false,
        ssg: false,
        rsc: _constants1.RSC_MODULE_TYPES.server,
        generateStaticParams: false,
        amp: false,
        runtime: undefined
    };
}

//# sourceMappingURL=get-page-static-info.js.map