"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return _default;
    }
});
const _icssutils = require("next/dist/compiled/icss-utils");
const _utils = require("../utils");
const plugin = (options = {})=>{
    return {
        postcssPlugin: 'postcss-icss-parser',
        async OnceExit (root) {
            const importReplacements = Object.create(null);
            const { icssImports, icssExports } = (0, _icssutils.extractICSS)(root);
            const imports = new Map();
            const tasks = [];
            // eslint-disable-next-line guard-for-in
            for(const url in icssImports){
                const tokens = icssImports[url];
                if (Object.keys(tokens).length === 0) {
                    continue;
                }
                let normalizedUrl = url;
                let prefix = '';
                const queryParts = normalizedUrl.split('!');
                if (queryParts.length > 1) {
                    normalizedUrl = queryParts.pop();
                    prefix = queryParts.join('!');
                }
                const request = (0, _utils.requestify)((0, _utils.normalizeUrl)(normalizedUrl, true), options.rootContext);
                const doResolve = async ()=>{
                    const { resolver, context } = options;
                    const resolvedUrl = await (0, _utils.resolveRequests)(resolver, context, [
                        ...new Set([
                            normalizedUrl,
                            request
                        ])
                    ]);
                    if (!resolvedUrl) {
                        return;
                    }
                    // eslint-disable-next-line consistent-return
                    return {
                        url: resolvedUrl,
                        prefix,
                        tokens
                    };
                };
                tasks.push(doResolve());
            }
            const results = await Promise.all(tasks);
            for(let index = 0; index <= results.length - 1; index++){
                const item = results[index];
                if (!item) {
                    continue;
                }
                const newUrl = item.prefix ? `${item.prefix}!${item.url}` : item.url;
                const importKey = newUrl;
                let importName = imports.get(importKey);
                if (!importName) {
                    importName = `___CSS_LOADER_ICSS_IMPORT_${imports.size}___`;
                    imports.set(importKey, importName);
                    options.imports.push({
                        type: 'icss_import',
                        importName,
                        url: options.urlHandler(newUrl),
                        icss: true,
                        index
                    });
                    options.api.push({
                        importName,
                        dedupe: true,
                        index
                    });
                }
                for (const [replacementIndex, token] of Object.keys(item.tokens).entries()){
                    const replacementName = `___CSS_LOADER_ICSS_IMPORT_${index}_REPLACEMENT_${replacementIndex}___`;
                    const localName = item.tokens[token];
                    importReplacements[token] = replacementName;
                    options.replacements.push({
                        replacementName,
                        importName,
                        localName
                    });
                }
            }
            if (Object.keys(importReplacements).length > 0) {
                (0, _icssutils.replaceSymbols)(root, importReplacements);
            }
            for (const name of Object.keys(icssExports)){
                const value = (0, _icssutils.replaceValueSymbols)(icssExports[name], importReplacements);
                options.exports.push({
                    name,
                    value
                });
            }
        }
    };
};
plugin.postcss = true;
const _default = plugin;

//# sourceMappingURL=postcss-icss-parser.js.map