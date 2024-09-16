"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    formatModuleTrace: null,
    getModuleTrace: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    formatModuleTrace: function() {
        return formatModuleTrace;
    },
    getModuleTrace: function() {
        return getModuleTrace;
    }
});
const _loaderutils3 = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/loader-utils3"));
const _path = require("path");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function formatModule(compiler, module1) {
    const relativePath = (0, _path.relative)(compiler.context, module1.resource).replace(/\?.+$/, "");
    return _loaderutils3.default.isUrlRequest(relativePath) ? _loaderutils3.default.urlToRequest(relativePath) : relativePath;
}
function formatModuleTrace(compiler, moduleTrace) {
    let importTrace = [];
    let firstExternalModule;
    for(let i = moduleTrace.length - 1; i >= 0; i--){
        const mod = moduleTrace[i];
        if (!mod.resource) continue;
        if (!mod.resource.includes("node_modules/")) {
            importTrace.unshift(formatModule(compiler, mod));
        } else {
            firstExternalModule = mod;
            break;
        }
    }
    let invalidImportMessage = "";
    if (firstExternalModule) {
        var _firstExternalModule_resourceResolveData_descriptionFileData, _firstExternalModule_resourceResolveData;
        const firstExternalPackageName = (_firstExternalModule_resourceResolveData = firstExternalModule.resourceResolveData) == null ? void 0 : (_firstExternalModule_resourceResolveData_descriptionFileData = _firstExternalModule_resourceResolveData.descriptionFileData) == null ? void 0 : _firstExternalModule_resourceResolveData_descriptionFileData.name;
        if (firstExternalPackageName === "styled-jsx") {
            invalidImportMessage += `\n\nThe error was caused by using 'styled-jsx' in '${importTrace[0]}'. It only works in a Client Component but none of its parents are marked with "use client", so they're Server Components by default.`;
        } else {
            let formattedExternalFile = firstExternalModule.resource.split("node_modules");
            formattedExternalFile = formattedExternalFile[formattedExternalFile.length - 1];
            invalidImportMessage += `\n\nThe error was caused by importing '${formattedExternalFile.slice(1)}' in '${importTrace[0]}'.`;
        }
    }
    return {
        lastInternalFileName: importTrace[0],
        invalidImportMessage,
        formattedModuleTrace: importTrace.map((mod)=>"  " + mod).join("\n")
    };
}
function getModuleTrace(module1, compilation, compiler) {
    // Get the module trace:
    // https://cs.github.com/webpack/webpack/blob/9fcaa243573005d6fdece9a3f8d89a0e8b399613/lib/stats/DefaultStatsFactoryPlugin.js#L414
    const visitedModules = new Set();
    const moduleTrace = [];
    let current = module1;
    let isPagesDir = false;
    while(current){
        if (visitedModules.has(current)) break;
        if (/[\\/]pages/.test(current.resource.replace(compiler.context, ""))) {
            isPagesDir = true;
        }
        visitedModules.add(current);
        moduleTrace.push(current);
        const origin = compilation.moduleGraph.getIssuer(current);
        if (!origin) break;
        current = origin;
    }
    return {
        moduleTrace,
        isPagesDir
    };
}

//# sourceMappingURL=getModuleTrace.js.map