"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    decodeFromBase64: null,
    encodeToBase64: null,
    getActionsFromBuildInfo: null,
    getLoaderModuleNamedExports: null,
    isCSSMod: null,
    isClientComponentEntryModule: null,
    regexCSS: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    decodeFromBase64: function() {
        return decodeFromBase64;
    },
    encodeToBase64: function() {
        return encodeToBase64;
    },
    getActionsFromBuildInfo: function() {
        return getActionsFromBuildInfo;
    },
    getLoaderModuleNamedExports: function() {
        return getLoaderModuleNamedExports;
    },
    isCSSMod: function() {
        return isCSSMod;
    },
    isClientComponentEntryModule: function() {
        return isClientComponentEntryModule;
    },
    regexCSS: function() {
        return regexCSS;
    }
});
const _constants = require("../../../shared/lib/constants");
const imageExtensions = [
    'jpg',
    'jpeg',
    'png',
    'webp',
    'avif',
    'ico',
    'svg'
];
const imageRegex = new RegExp(`\\.(${imageExtensions.join('|')})$`);
// Determine if the whole module is client action, 'use server' in nested closure in the client module
function isActionClientLayerModule(mod) {
    const rscInfo = mod.buildInfo.rsc;
    return !!((rscInfo == null ? void 0 : rscInfo.actions) && (rscInfo == null ? void 0 : rscInfo.type) === _constants.RSC_MODULE_TYPES.client);
}
function isClientComponentEntryModule(mod) {
    const rscInfo = mod.buildInfo.rsc;
    const hasClientDirective = rscInfo == null ? void 0 : rscInfo.isClientRef;
    const isActionLayerEntry = isActionClientLayerModule(mod);
    return hasClientDirective || isActionLayerEntry || imageRegex.test(mod.resource);
}
const regexCSS = /\.(css|scss|sass)(\?.*)?$/;
function isCSSMod(mod) {
    var _mod_loaders;
    return !!(mod.type === 'css/mini-extract' || mod.resource && regexCSS.test(mod.resource) || ((_mod_loaders = mod.loaders) == null ? void 0 : _mod_loaders.some(({ loader })=>loader.includes('next-style-loader/index.js') || loader.includes('mini-css-extract-plugin/loader.js') || loader.includes('@vanilla-extract/webpack-plugin/loader/'))));
}
function getActionsFromBuildInfo(mod) {
    var _mod_buildInfo_rsc, _mod_buildInfo;
    return (_mod_buildInfo = mod.buildInfo) == null ? void 0 : (_mod_buildInfo_rsc = _mod_buildInfo.rsc) == null ? void 0 : _mod_buildInfo_rsc.actionIds;
}
function encodeToBase64(obj) {
    return Buffer.from(JSON.stringify(obj)).toString('base64');
}
function decodeFromBase64(str) {
    return JSON.parse(Buffer.from(str, 'base64').toString('utf8'));
}
async function getLoaderModuleNamedExports(resourcePath, context) {
    var _mod_dependencies;
    const mod = await new Promise((res, rej)=>{
        context.loadModule(resourcePath, (err, _source, _sourceMap, module1)=>{
            if (err) {
                return rej(err);
            }
            res(module1);
        });
    });
    const exportNames = ((_mod_dependencies = mod.dependencies) == null ? void 0 : _mod_dependencies.filter((dep)=>{
        return [
            'HarmonyExportImportedSpecifierDependency',
            'HarmonyExportSpecifierDependency'
        ].includes(dep.constructor.name) && 'name' in dep && dep.name !== 'default';
    }).map((dep)=>{
        return dep.name;
    })) || [];
    return exportNames;
}

//# sourceMappingURL=utils.js.map