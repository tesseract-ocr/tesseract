import { promises as fs } from 'fs';
import { LRUCache } from '../../server/lib/lru-cache';
import { extractExportedConstValue, UnsupportedValueError } from './extract-const-value';
import { parseModule } from './parse-module';
import * as Log from '../output/log';
import { SERVER_RUNTIME } from '../../lib/constants';
import { tryToParsePath } from '../../lib/try-to-parse-path';
import { isAPIRoute } from '../../lib/is-api-route';
import { isEdgeRuntime } from '../../lib/is-edge-runtime';
import { RSC_MODULE_TYPES } from '../../shared/lib/constants';
import { PAGE_TYPES } from '../../lib/page-types';
import { AppSegmentConfigSchemaKeys, parseAppSegmentConfig } from '../segment-config/app/app-segment-config';
import { reportZodError } from '../../shared/lib/zod';
import { PagesSegmentConfigSchemaKeys, parsePagesSegmentConfig } from '../segment-config/pages/pages-segment-config';
import { MiddlewareConfigInputSchema, SourceSchema } from '../segment-config/middleware/middleware-config';
import { normalizeAppPath } from '../../shared/lib/router/utils/app-paths';
import { normalizePagePath } from '../../shared/lib/page-path/normalize-page-path';
const PARSE_PATTERN = /(?<!(_jsx|jsx-))runtime|preferredRegion|getStaticProps|getServerSideProps|generateStaticParams|export const|generateImageMetadata|generateSitemaps/;
const CLIENT_MODULE_LABEL = /\/\* __next_internal_client_entry_do_not_use__ ([^ ]*) (cjs|auto) \*\//;
const ACTION_MODULE_LABEL = /\/\* __next_internal_action_entry_do_not_use__ (\{[^}]+\}) \*\//;
const CLIENT_DIRECTIVE = 'use client';
const SERVER_ACTION_DIRECTIVE = 'use server';
export function getRSCModuleInformation(source, isReactServerLayer) {
    const actionsJson = source.match(ACTION_MODULE_LABEL);
    const parsedActionsMeta = actionsJson ? JSON.parse(actionsJson[1]) : undefined;
    const actions = parsedActionsMeta ? Object.values(parsedActionsMeta) : undefined;
    const clientInfoMatch = source.match(CLIENT_MODULE_LABEL);
    const isClientRef = !!clientInfoMatch;
    if (!isReactServerLayer) {
        return {
            type: RSC_MODULE_TYPES.client,
            actions,
            actionIds: parsedActionsMeta,
            isClientRef
        };
    }
    const clientRefsString = clientInfoMatch == null ? void 0 : clientInfoMatch[1];
    const clientRefs = clientRefsString ? clientRefsString.split(',') : [];
    const clientEntryType = clientInfoMatch == null ? void 0 : clientInfoMatch[2];
    const type = clientInfoMatch ? RSC_MODULE_TYPES.client : RSC_MODULE_TYPES.server;
    return {
        type,
        actions,
        actionIds: parsedActionsMeta,
        clientRefs,
        clientEntryType,
        isClientRef
    };
}
/**
 * Receives a parsed AST from SWC and checks if it belongs to a module that
 * requires a runtime to be specified. Those are:
 *   - Modules with `export function getStaticProps | getServerSideProps`
 *   - Modules with `export { getStaticProps | getServerSideProps } <from ...>`
 *   - Modules with `export const runtime = ...`
 */ function checkExports(ast, expectedExports, page) {
    const exportsSet = new Set([
        'getStaticProps',
        'getServerSideProps',
        'generateImageMetadata',
        'generateSitemaps',
        'generateStaticParams'
    ]);
    if (!Array.isArray(ast == null ? void 0 : ast.body)) {
        return {};
    }
    try {
        let getStaticProps = false;
        let getServerSideProps = false;
        let generateImageMetadata = false;
        let generateSitemaps = false;
        let generateStaticParams = false;
        let exports = new Set();
        let directives = new Set();
        let hasLeadingNonDirectiveNode = false;
        for (const node of ast.body){
            var _node_declaration, _node_declaration1, _node_declaration_identifier, _node_declaration2;
            // There should be no non-string literals nodes before directives
            if (node.type === 'ExpressionStatement' && node.expression.type === 'StringLiteral') {
                if (!hasLeadingNonDirectiveNode) {
                    const directive = node.expression.value;
                    if (CLIENT_DIRECTIVE === directive) {
                        directives.add('client');
                    }
                    if (SERVER_ACTION_DIRECTIVE === directive) {
                        directives.add('server');
                    }
                }
            } else {
                hasLeadingNonDirectiveNode = true;
            }
            if (node.type === 'ExportDeclaration' && ((_node_declaration = node.declaration) == null ? void 0 : _node_declaration.type) === 'VariableDeclaration') {
                var _node_declaration3;
                for (const declaration of (_node_declaration3 = node.declaration) == null ? void 0 : _node_declaration3.declarations){
                    if (expectedExports.includes(declaration.id.value)) {
                        exports.add(declaration.id.value);
                    }
                }
            }
            if (node.type === 'ExportDeclaration' && ((_node_declaration1 = node.declaration) == null ? void 0 : _node_declaration1.type) === 'FunctionDeclaration' && exportsSet.has((_node_declaration_identifier = node.declaration.identifier) == null ? void 0 : _node_declaration_identifier.value)) {
                const id = node.declaration.identifier.value;
                getServerSideProps = id === 'getServerSideProps';
                getStaticProps = id === 'getStaticProps';
                generateImageMetadata = id === 'generateImageMetadata';
                generateSitemaps = id === 'generateSitemaps';
                generateStaticParams = id === 'generateStaticParams';
            }
            if (node.type === 'ExportDeclaration' && ((_node_declaration2 = node.declaration) == null ? void 0 : _node_declaration2.type) === 'VariableDeclaration') {
                var _node_declaration_declarations_, _node_declaration4;
                const id = (_node_declaration4 = node.declaration) == null ? void 0 : (_node_declaration_declarations_ = _node_declaration4.declarations[0]) == null ? void 0 : _node_declaration_declarations_.id.value;
                if (exportsSet.has(id)) {
                    getServerSideProps = id === 'getServerSideProps';
                    getStaticProps = id === 'getStaticProps';
                    generateImageMetadata = id === 'generateImageMetadata';
                    generateSitemaps = id === 'generateSitemaps';
                    generateStaticParams = id === 'generateStaticParams';
                }
            }
            if (node.type === 'ExportNamedDeclaration') {
                for (const specifier of node.specifiers){
                    var _specifier_orig;
                    if (specifier.type === 'ExportSpecifier' && ((_specifier_orig = specifier.orig) == null ? void 0 : _specifier_orig.type) === 'Identifier') {
                        const value = specifier.orig.value;
                        if (!getServerSideProps && value === 'getServerSideProps') {
                            getServerSideProps = true;
                        }
                        if (!getStaticProps && value === 'getStaticProps') {
                            getStaticProps = true;
                        }
                        if (!generateImageMetadata && value === 'generateImageMetadata') {
                            generateImageMetadata = true;
                        }
                        if (!generateSitemaps && value === 'generateSitemaps') {
                            generateSitemaps = true;
                        }
                        if (!generateStaticParams && value === 'generateStaticParams') {
                            generateStaticParams = true;
                        }
                        if (expectedExports.includes(value) && !exports.has(value)) {
                            // An export was found that was actually a re-export, and not a
                            // literal value. We should warn here.
                            Log.warn(`Next.js can't recognize the exported \`${value}\` field in "${page}", it may be re-exported from another file. The default config will be used instead.`);
                        }
                    }
                }
            }
        }
        return {
            getStaticProps,
            getServerSideProps,
            generateImageMetadata,
            generateSitemaps,
            generateStaticParams,
            directives,
            exports
        };
    } catch  {}
    return {};
}
async function tryToReadFile(filePath, shouldThrow) {
    try {
        return await fs.readFile(filePath, {
            encoding: 'utf8'
        });
    } catch (error) {
        if (shouldThrow) {
            error.message = `Next.js ERROR: Failed to read file ${filePath}:\n${error.message}`;
            throw error;
        }
    }
}
/**
 * @internal - required to exclude zod types from the build
 */ export function getMiddlewareMatchers(matcherOrMatchers, nextConfig) {
    const matchers = Array.isArray(matcherOrMatchers) ? matcherOrMatchers : [
        matcherOrMatchers
    ];
    const { i18n } = nextConfig;
    return matchers.map((matcher)=>{
        matcher = typeof matcher === 'string' ? {
            source: matcher
        } : matcher;
        const originalSource = matcher.source;
        let { source, ...rest } = matcher;
        const isRoot = source === '/';
        if ((i18n == null ? void 0 : i18n.locales) && matcher.locale !== false) {
            source = `/:nextInternalLocale((?!_next/)[^/.]{1,})${isRoot ? '' : source}`;
        }
        source = `/:nextData(_next/data/[^/]{1,})?${source}${isRoot ? `(${nextConfig.i18n ? '|\\.json|' : ''}/?index|/?index\\.json)?` : '{(\\.json)}?'}`;
        if (nextConfig.basePath) {
            source = `${nextConfig.basePath}${source}`;
        }
        // Validate that the source is still.
        const result = SourceSchema.safeParse(source);
        if (!result.success) {
            reportZodError('Failed to parse middleware source', result.error);
            // We need to exit here because middleware being built occurs before we
            // finish setting up the server. Exiting here is the only way to ensure
            // that we don't hang.
            process.exit(1);
        }
        return {
            ...rest,
            // We know that parsed.regexStr is not undefined because we already
            // checked that the source is valid.
            regexp: tryToParsePath(result.data).regexStr,
            originalSource: originalSource || source
        };
    });
}
function parseMiddlewareConfig(page, rawConfig, nextConfig) {
    // If there's no config to parse, then return nothing.
    if (typeof rawConfig !== 'object' || !rawConfig) return {};
    const input = MiddlewareConfigInputSchema.safeParse(rawConfig);
    if (!input.success) {
        reportZodError(`${page} contains invalid middleware config`, input.error);
        // We need to exit here because middleware being built occurs before we
        // finish setting up the server. Exiting here is the only way to ensure
        // that we don't hang.
        process.exit(1);
    }
    const config = {};
    if (input.data.matcher) {
        config.matchers = getMiddlewareMatchers(input.data.matcher, nextConfig);
    }
    if (input.data.unstable_allowDynamic) {
        config.unstable_allowDynamic = Array.isArray(input.data.unstable_allowDynamic) ? input.data.unstable_allowDynamic : [
            input.data.unstable_allowDynamic
        ];
    }
    if (input.data.regions) {
        config.regions = input.data.regions;
    }
    return config;
}
const apiRouteWarnings = new LRUCache(250);
function warnAboutExperimentalEdge(apiRoute) {
    if (process.env.NODE_ENV === 'production' && process.env.NEXT_PRIVATE_BUILD_WORKER === '1') {
        return;
    }
    if (apiRouteWarnings.has(apiRoute)) {
        return;
    }
    Log.warn(apiRoute ? `${apiRoute} provided runtime 'experimental-edge'. It can be updated to 'edge' instead.` : `You are using an experimental edge runtime, the API might change.`);
    apiRouteWarnings.set(apiRoute, 1);
}
export let hadUnsupportedValue = false;
const warnedUnsupportedValueMap = new LRUCache(250, ()=>1);
function warnAboutUnsupportedValue(pageFilePath, page, error) {
    hadUnsupportedValue = true;
    const isProductionBuild = process.env.NODE_ENV === 'production';
    if (// we only log for the server compilation so it's not
    // duplicated due to webpack build worker having fresh
    // module scope for each compiler
    process.env.NEXT_COMPILER_NAME !== 'server' || isProductionBuild && warnedUnsupportedValueMap.has(pageFilePath)) {
        return;
    }
    warnedUnsupportedValueMap.set(pageFilePath, true);
    const message = `Next.js can't recognize the exported \`config\` field in ` + (page ? `route "${page}"` : `"${pageFilePath}"`) + ':\n' + error.message + (error.path ? ` at "${error.path}"` : '') + '.\n' + 'Read More - https://nextjs.org/docs/messages/invalid-page-config';
    // for a build we use `Log.error` instead of throwing
    // so that all errors can be logged before exiting the process
    if (isProductionBuild) {
        Log.error(message);
    } else {
        throw new Error(message);
    }
}
export async function getAppPageStaticInfo({ pageFilePath, nextConfig, isDev, page }) {
    const content = await tryToReadFile(pageFilePath, !isDev);
    if (!content || !PARSE_PATTERN.test(content)) {
        return {
            type: PAGE_TYPES.APP,
            config: undefined,
            runtime: undefined,
            preferredRegion: undefined,
            maxDuration: undefined
        };
    }
    const ast = await parseModule(pageFilePath, content);
    const { generateStaticParams, generateImageMetadata, generateSitemaps, exports, directives } = checkExports(ast, AppSegmentConfigSchemaKeys, page);
    const { type: rsc } = getRSCModuleInformation(content, true);
    const exportedConfig = {};
    if (exports) {
        for (const property of exports){
            try {
                exportedConfig[property] = extractExportedConstValue(ast, property);
            } catch (e) {
                if (e instanceof UnsupportedValueError) {
                    warnAboutUnsupportedValue(pageFilePath, page, e);
                }
            }
        }
    }
    try {
        exportedConfig.config = extractExportedConstValue(ast, 'config');
    } catch (e) {
        if (e instanceof UnsupportedValueError) {
            warnAboutUnsupportedValue(pageFilePath, page, e);
        }
    // `export config` doesn't exist, or other unknown error thrown by swc, silence them
    }
    const route = normalizeAppPath(page);
    const config = parseAppSegmentConfig(exportedConfig, route);
    // Prevent edge runtime and generateStaticParams in the same file.
    if (isEdgeRuntime(config.runtime) && generateStaticParams) {
        throw new Error(`Page "${page}" cannot use both \`export const runtime = 'edge'\` and export \`generateStaticParams\`.`);
    }
    // Prevent use client and generateStaticParams in the same file.
    if ((directives == null ? void 0 : directives.has('client')) && generateStaticParams) {
        throw new Error(`Page "${page}" cannot use both "use client" and export function "generateStaticParams()".`);
    }
    return {
        type: PAGE_TYPES.APP,
        rsc,
        generateImageMetadata,
        generateSitemaps,
        generateStaticParams,
        config,
        middleware: parseMiddlewareConfig(page, exportedConfig.config, nextConfig),
        runtime: config.runtime,
        preferredRegion: config.preferredRegion,
        maxDuration: config.maxDuration
    };
}
export async function getPagesPageStaticInfo({ pageFilePath, nextConfig, isDev, page }) {
    var _config_config, _config_config1, _config_config2, _config_config3;
    const content = await tryToReadFile(pageFilePath, !isDev);
    if (!content || !PARSE_PATTERN.test(content)) {
        return {
            type: PAGE_TYPES.PAGES,
            config: undefined,
            runtime: undefined,
            preferredRegion: undefined,
            maxDuration: undefined
        };
    }
    const ast = await parseModule(pageFilePath, content);
    const { getServerSideProps, getStaticProps, exports } = checkExports(ast, PagesSegmentConfigSchemaKeys, page);
    const { type: rsc } = getRSCModuleInformation(content, true);
    const exportedConfig = {};
    if (exports) {
        for (const property of exports){
            try {
                exportedConfig[property] = extractExportedConstValue(ast, property);
            } catch (e) {
                if (e instanceof UnsupportedValueError) {
                    warnAboutUnsupportedValue(pageFilePath, page, e);
                }
            }
        }
    }
    try {
        exportedConfig.config = extractExportedConstValue(ast, 'config');
    } catch (e) {
        if (e instanceof UnsupportedValueError) {
            warnAboutUnsupportedValue(pageFilePath, page, e);
        }
    // `export config` doesn't exist, or other unknown error thrown by swc, silence them
    }
    // Validate the config.
    const route = normalizePagePath(page);
    const config = parsePagesSegmentConfig(exportedConfig, route);
    const isAnAPIRoute = isAPIRoute(route);
    const resolvedRuntime = isEdgeRuntime(config.runtime ?? ((_config_config = config.config) == null ? void 0 : _config_config.runtime)) || getServerSideProps || getStaticProps ? config.runtime ?? ((_config_config1 = config.config) == null ? void 0 : _config_config1.runtime) : undefined;
    if (resolvedRuntime === SERVER_RUNTIME.experimentalEdge) {
        warnAboutExperimentalEdge(isAnAPIRoute ? page : null);
    }
    if (resolvedRuntime === SERVER_RUNTIME.edge && page && !isAnAPIRoute) {
        const message = `Page ${page} provided runtime 'edge', the edge runtime for rendering is currently experimental. Use runtime 'experimental-edge' instead.`;
        if (isDev) {
            Log.error(message);
        } else {
            throw new Error(message);
        }
    }
    return {
        type: PAGE_TYPES.PAGES,
        getStaticProps,
        getServerSideProps,
        rsc,
        config,
        middleware: parseMiddlewareConfig(page, exportedConfig.config, nextConfig),
        runtime: resolvedRuntime,
        preferredRegion: (_config_config2 = config.config) == null ? void 0 : _config_config2.regions,
        maxDuration: config.maxDuration ?? ((_config_config3 = config.config) == null ? void 0 : _config_config3.maxDuration)
    };
}
/**
 * For a given pageFilePath and nextConfig, if the config supports it, this
 * function will read the file and return the runtime that should be used.
 * It will look into the file content only if the page *requires* a runtime
 * to be specified, that is, when gSSP or gSP is used.
 * Related discussion: https://github.com/vercel/next.js/discussions/34179
 */ export async function getPageStaticInfo(params) {
    if (params.pageType === PAGE_TYPES.APP) {
        return getAppPageStaticInfo(params);
    }
    return getPagesPageStaticInfo(params);
}

//# sourceMappingURL=get-page-static-info.js.map