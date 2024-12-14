"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    LightningCssLoader: null,
    raw: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    LightningCssLoader: function() {
        return LightningCssLoader;
    },
    raw: function() {
        return raw;
    }
});
const _utils = require("./utils");
const _codegen = require("./codegen");
const _utils1 = require("../../css-loader/src/utils");
const _stringifyrequest = require("../../../stringify-request");
const _interface = require("./interface");
const encoder = new TextEncoder();
function createUrlAndImportVisitor(visitorOptions, apis, imports, replacements, replacedUrls, replacedImportUrls) {
    const importUrlToNameMap = new Map();
    let hasUrlImportHelper = false;
    const urlToNameMap = new Map();
    const urlToReplacementMap = new Map();
    let urlIndex = -1;
    let importUrlIndex = -1;
    function handleUrl(u) {
        let url = u.url;
        const needKeep = visitorOptions.urlFilter(url);
        if (!needKeep) {
            return u;
        }
        if ((0, _utils1.isDataUrl)(url)) {
            return u;
        }
        urlIndex++;
        replacedUrls.set(urlIndex, url);
        url = `__NEXT_LIGHTNINGCSS_LOADER_URL_REPLACE_${urlIndex}__`;
        const [, query, hashOrQuery] = url.split(/(\?)?#/, 3);
        const queryParts = url.split('!');
        let prefix;
        if (queryParts.length > 1) {
            url = queryParts.pop();
            prefix = queryParts.join('!');
        }
        let hash = query ? '?' : '';
        hash += hashOrQuery ? `#${hashOrQuery}` : '';
        if (!hasUrlImportHelper) {
            imports.push({
                type: 'get_url_import',
                importName: '___CSS_LOADER_GET_URL_IMPORT___',
                url: JSON.stringify(require.resolve('../../css-loader/src/runtime/getUrl.js')),
                index: -1
            });
            hasUrlImportHelper = true;
        }
        const newUrl = prefix ? `${prefix}!${url}` : url;
        let importName = urlToNameMap.get(newUrl);
        if (!importName) {
            importName = `___CSS_LOADER_URL_IMPORT_${urlToNameMap.size}___`;
            urlToNameMap.set(newUrl, importName);
            imports.push({
                type: 'url',
                importName,
                url: JSON.stringify(newUrl),
                index: urlIndex
            });
        }
        // This should be true for string-urls in image-set
        const needQuotes = false;
        const replacementKey = JSON.stringify({
            newUrl,
            hash,
            needQuotes
        });
        let replacementName = urlToReplacementMap.get(replacementKey);
        if (!replacementName) {
            replacementName = `___CSS_LOADER_URL_REPLACEMENT_${urlToReplacementMap.size}___`;
            urlToReplacementMap.set(replacementKey, replacementName);
            replacements.push({
                replacementName,
                importName,
                hash,
                needQuotes
            });
        }
        return {
            loc: u.loc,
            url: replacementName
        };
    }
    return {
        Rule: {
            import (node) {
                if (visitorOptions.importFilter) {
                    const needKeep = visitorOptions.importFilter(node.value.url, node.value.media);
                    if (!needKeep) {
                        return node;
                    }
                }
                let url = node.value.url;
                importUrlIndex++;
                replacedImportUrls.set(importUrlIndex, url);
                url = `__NEXT_LIGHTNINGCSS_LOADER_IMPORT_URL_REPLACE_${importUrlIndex}__`;
                // TODO: Use identical logic as valueParser.stringify()
                const media = node.value.media.mediaQueries.length ? JSON.stringify(node.value.media.mediaQueries) : undefined;
                const isRequestable = (0, _utils1.isUrlRequestable)(url);
                let prefix;
                if (isRequestable) {
                    const queryParts = url.split('!');
                    if (queryParts.length > 1) {
                        url = queryParts.pop();
                        prefix = queryParts.join('!');
                    }
                }
                if (!isRequestable) {
                    apis.push({
                        url,
                        media
                    });
                    // Bug of lightningcss
                    return {
                        type: 'ignored',
                        value: ''
                    };
                }
                const newUrl = prefix ? `${prefix}!${url}` : url;
                let importName = importUrlToNameMap.get(newUrl);
                if (!importName) {
                    importName = `___CSS_LOADER_AT_RULE_IMPORT_${importUrlToNameMap.size}___`;
                    importUrlToNameMap.set(newUrl, importName);
                    const importUrl = visitorOptions.urlHandler(newUrl);
                    imports.push({
                        type: 'rule_import',
                        importName,
                        url: importUrl
                    });
                }
                apis.push({
                    importName,
                    media
                });
                // Bug of lightningcss
                return {
                    type: 'ignored',
                    value: ''
                };
            }
        },
        Url (node) {
            return handleUrl(node);
        }
    };
}
function createIcssVisitor({ apis, imports, replacements, replacedUrls, urlHandler }) {
    let index = -1;
    let replacementIndex = -1;
    return {
        Declaration: {
            composes (node) {
                if (node.property === 'unparsed') {
                    return;
                }
                const specifier = node.value.from;
                if ((specifier == null ? void 0 : specifier.type) !== 'file') {
                    return;
                }
                let url = specifier.value;
                if (!url) {
                    return;
                }
                index++;
                replacedUrls.set(index, url);
                url = `__NEXT_LIGHTNINGCSS_LOADER_ICSS_URL_REPLACE_${index}__`;
                const importName = `___CSS_LOADER_ICSS_IMPORT_${imports.length}___`;
                imports.push({
                    type: 'icss_import',
                    importName,
                    icss: true,
                    url: urlHandler(url),
                    index
                });
                apis.push({
                    importName,
                    dedupe: true,
                    index
                });
                const newNames = [];
                for (const localName of node.value.names){
                    replacementIndex++;
                    const replacementName = `___CSS_LOADER_ICSS_IMPORT_${index}_REPLACEMENT_${replacementIndex}___`;
                    replacements.push({
                        replacementName,
                        importName,
                        localName
                    });
                    newNames.push(replacementName);
                }
                return {
                    property: 'composes',
                    value: {
                        loc: node.value.loc,
                        names: newNames,
                        from: specifier
                    }
                };
            }
        }
    };
}
const LOADER_NAME = `lightningcss-loader`;
async function LightningCssLoader(source, prevMap) {
    var _options_modules;
    const done = this.async();
    const options = this.getOptions();
    const { implementation, targets: userTargets, ...opts } = options;
    options.modules ??= {};
    if (implementation && typeof implementation.transformCss !== 'function') {
        done(new TypeError(`[${LOADER_NAME}]: options.implementation.transformCss must be an 'lightningcss' transform function. Received ${typeof implementation.transformCss}`));
        return;
    }
    if (options.postcss) {
        var _postcssWithPlugins_plugins;
        const { postcssWithPlugins } = await options.postcss();
        if ((postcssWithPlugins == null ? void 0 : (_postcssWithPlugins_plugins = postcssWithPlugins.plugins) == null ? void 0 : _postcssWithPlugins_plugins.length) > 0) {
            throw new Error(`[${LOADER_NAME}]: experimental.useLightningcss does not work with postcss plugins. Please remove 'useLightningcss: true' from your configuration.`);
        }
    }
    const exports1 = [];
    const imports = [];
    const icssImports = [];
    const apis = [];
    const replacements = [];
    if (((_options_modules = options.modules) == null ? void 0 : _options_modules.exportOnlyLocals) !== true) {
        imports.unshift({
            type: 'api_import',
            importName: '___CSS_LOADER_API_IMPORT___',
            url: (0, _stringifyrequest.stringifyRequest)(this, require.resolve('../../css-loader/src/runtime/api'))
        });
    }
    const { loadBindings } = require('next/dist/build/swc');
    const transform = (implementation == null ? void 0 : implementation.transformCss) ?? (await loadBindings()).css.lightning.transform;
    const replacedUrls = new Map();
    const icssReplacedUrls = new Map();
    const replacedImportUrls = new Map();
    const urlImportVisitor = createUrlAndImportVisitor({
        urlHandler: (url)=>(0, _stringifyrequest.stringifyRequest)(this, (0, _utils1.getPreRequester)(this)(options.importLoaders ?? 0) + url),
        urlFilter: (0, _utils1.getFilter)(options.url, this.resourcePath),
        importFilter: (0, _utils1.getFilter)(options.import, this.resourcePath),
        context: this.context
    }, apis, imports, replacements, replacedUrls, replacedImportUrls);
    const icssVisitor = createIcssVisitor({
        apis,
        imports: icssImports,
        replacements,
        replacedUrls: icssReplacedUrls,
        urlHandler: (url)=>(0, _stringifyrequest.stringifyRequest)(this, (0, _utils1.getPreRequester)(this)(options.importLoaders) + url)
    });
    // This works by returned visitors are not conflicting.
    // naive workaround for composeVisitors, as we do not directly depends on lightningcss's npm pkg
    // but next-swc provides bindings
    const visitor = {
        ...urlImportVisitor,
        ...icssVisitor
    };
    try {
        const { code, map, exports: moduleExports } = transform({
            ...opts,
            visitor,
            cssModules: options.modules ? {
                pattern: process.env.__NEXT_TEST_MODE ? '[name]__[local]' : '[name]__[hash]__[local]'
            } : undefined,
            filename: this.resourcePath,
            code: encoder.encode(source),
            sourceMap: this.sourceMap,
            targets: (0, _utils.getTargets)({
                targets: userTargets,
                key: _interface.ECacheKey.loader
            }),
            inputSourceMap: this.sourceMap && prevMap ? JSON.stringify(prevMap) : undefined,
            include: 1
        });
        let cssCodeAsString = code.toString();
        if (moduleExports) {
            for(const name in moduleExports){
                if (Object.prototype.hasOwnProperty.call(moduleExports, name)) {
                    const v = moduleExports[name];
                    let value = v.name;
                    for (const compose of v.composes){
                        value += ` ${compose.name}`;
                    }
                    exports1.push({
                        name,
                        value
                    });
                }
            }
        }
        if (replacedUrls.size !== 0) {
            const urlResolver = this.getResolve({
                conditionNames: [
                    'asset'
                ],
                mainFields: [
                    'asset'
                ],
                mainFiles: [],
                extensions: []
            });
            for (const [index, url] of replacedUrls.entries()){
                const [pathname] = url.split(/(\?)?#/, 3);
                const request = (0, _utils1.requestify)(pathname, this.rootContext);
                const resolvedUrl = await (0, _utils1.resolveRequests)(urlResolver, this.context, [
                    ...new Set([
                        request,
                        url
                    ])
                ]);
                for (const importItem of imports){
                    importItem.url = importItem.url.replace(`__NEXT_LIGHTNINGCSS_LOADER_URL_REPLACE_${index}__`, resolvedUrl ?? url);
                }
            }
        }
        if (replacedImportUrls.size !== 0) {
            const importResolver = this.getResolve({
                conditionNames: [
                    'style'
                ],
                extensions: [
                    '.css'
                ],
                mainFields: [
                    'css',
                    'style',
                    'main',
                    '...'
                ],
                mainFiles: [
                    'index',
                    '...'
                ],
                restrictions: [
                    /\.css$/i
                ]
            });
            for (const [index, url] of replacedImportUrls.entries()){
                const [pathname] = url.split(/(\?)?#/, 3);
                const request = (0, _utils1.requestify)(pathname, this.rootContext);
                const resolvedUrl = await (0, _utils1.resolveRequests)(importResolver, this.context, [
                    ...new Set([
                        request,
                        url
                    ])
                ]);
                for (const importItem of imports){
                    importItem.url = importItem.url.replace(`__NEXT_LIGHTNINGCSS_LOADER_IMPORT_URL_REPLACE_${index}__`, resolvedUrl ?? url);
                }
            }
        }
        if (icssReplacedUrls.size !== 0) {
            const icssResolver = this.getResolve({
                conditionNames: [
                    'style'
                ],
                extensions: [],
                mainFields: [
                    'css',
                    'style',
                    'main',
                    '...'
                ],
                mainFiles: [
                    'index',
                    '...'
                ]
            });
            for (const [index, url] of icssReplacedUrls.entries()){
                const [pathname] = url.split(/(\?)?#/, 3);
                const request = (0, _utils1.requestify)(pathname, this.rootContext);
                const resolvedUrl = await (0, _utils1.resolveRequests)(icssResolver, this.context, [
                    ...new Set([
                        url,
                        request
                    ])
                ]);
                for (const importItem of icssImports){
                    importItem.url = importItem.url.replace(`__NEXT_LIGHTNINGCSS_LOADER_ICSS_URL_REPLACE_${index}__`, resolvedUrl ?? url);
                }
            }
        }
        imports.push(...icssImports);
        const importCode = (0, _codegen.getImportCode)(imports, options);
        const moduleCode = (0, _codegen.getModuleCode)({
            css: cssCodeAsString,
            map
        }, apis, replacements, options, this);
        const exportCode = (0, _codegen.getExportCode)(exports1, replacements, options);
        const esCode = `${importCode}${moduleCode}${exportCode}`;
        done(null, esCode, map && JSON.parse(map.toString()));
    } catch (error) {
        console.error('lightningcss-loader error', error);
        done(error);
    }
}
const raw = true;

//# sourceMappingURL=loader.js.map