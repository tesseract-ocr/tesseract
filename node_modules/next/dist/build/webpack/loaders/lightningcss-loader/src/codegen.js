"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getExportCode: null,
    getImportCode: null,
    getModuleCode: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getExportCode: function() {
        return getExportCode;
    },
    getImportCode: function() {
        return getImportCode;
    },
    getModuleCode: function() {
        return getModuleCode;
    }
});
const _camelcase = /*#__PURE__*/ _interop_require_default(require("../../css-loader/src/camelcase"));
const _utils = require("../../css-loader/src/utils");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function getImportCode(imports, options) {
    let code = "";
    for (const item of imports){
        const { importName, url, icss } = item;
        if (options.esModule) {
            if (icss && options.modules.namedExport) {
                code += `import ${options.modules.exportOnlyLocals ? "" : `${importName}, `}* as ${importName}_NAMED___ from ${url};\n`;
            } else {
                code += `import ${importName} from ${url};\n`;
            }
        } else {
            code += `var ${importName} = require(${url});\n`;
        }
    }
    return code ? `// Imports\n${code}` : "";
}
function getModuleCode(result, api, replacements, options, loaderContext) {
    if (options.modules.exportOnlyLocals === true) {
        return "";
    }
    const sourceMapValue = options.sourceMap ? `,${(0, _utils.normalizeSourceMapForRuntime)(result.map, loaderContext)}` : "";
    let code = JSON.stringify(result.css);
    let beforeCode = `var ___CSS_LOADER_EXPORT___ = ___CSS_LOADER_API_IMPORT___(${options.sourceMap});\n`;
    for (const item of api){
        const { url, media, dedupe } = item;
        beforeCode += url ? `___CSS_LOADER_EXPORT___.push([module.id, ${JSON.stringify(`@import url(${url});`)}${media ? `, ${JSON.stringify(media)}` : ""}]);\n` : `___CSS_LOADER_EXPORT___.i(${item.importName}${media ? `, ${JSON.stringify(media)}` : dedupe ? ', ""' : ""}${dedupe ? ", true" : ""});\n`;
    }
    for (const item of replacements){
        const { replacementName, importName, localName } = item;
        if (localName) {
            code = code.replace(new RegExp(replacementName, "g"), ()=>options.modules.namedExport ? `" + ${importName}_NAMED___[${JSON.stringify((0, _camelcase.default)(localName))}] + "` : `" + ${importName}.locals[${JSON.stringify(localName)}] + "`);
        } else {
            const { hash, needQuotes } = item;
            const getUrlOptions = [
                ...hash ? [
                    `hash: ${JSON.stringify(hash)}`
                ] : [],
                ...needQuotes ? "needQuotes: true" : []
            ];
            const preparedOptions = getUrlOptions.length > 0 ? `, { ${getUrlOptions.join(", ")} }` : "";
            beforeCode += `var ${replacementName} = ___CSS_LOADER_GET_URL_IMPORT___(${importName}${preparedOptions});\n`;
            code = code.replace(new RegExp(replacementName, "g"), ()=>`" + ${replacementName} + "`);
        }
    }
    return `${beforeCode}// Module\n___CSS_LOADER_EXPORT___.push([module.id, ${code}, ""${sourceMapValue}]);\n`;
}
function getExportCode(exports1, replacements, options) {
    let code = "// Exports\n";
    let localsCode = "";
    const addExportToLocalsCode = (name, value)=>{
        if (options.modules.namedExport) {
            localsCode += `export const ${(0, _camelcase.default)(name)} = ${JSON.stringify(value)};\n`;
        } else {
            if (localsCode) {
                localsCode += `,\n`;
            }
            localsCode += `\t${JSON.stringify(name)}: ${JSON.stringify(value)}`;
        }
    };
    for (const { name, value } of exports1){
        switch(options.modules.exportLocalsConvention){
            case "camelCase":
                {
                    addExportToLocalsCode(name, value);
                    const modifiedName = (0, _camelcase.default)(name);
                    if (modifiedName !== name) {
                        addExportToLocalsCode(modifiedName, value);
                    }
                    break;
                }
            case "camelCaseOnly":
                {
                    addExportToLocalsCode((0, _camelcase.default)(name), value);
                    break;
                }
            case "dashes":
                {
                    addExportToLocalsCode(name, value);
                    const modifiedName = (0, _utils.dashesCamelCase)(name);
                    if (modifiedName !== name) {
                        addExportToLocalsCode(modifiedName, value);
                    }
                    break;
                }
            case "dashesOnly":
                {
                    addExportToLocalsCode((0, _utils.dashesCamelCase)(name), value);
                    break;
                }
            case "asIs":
            default:
                addExportToLocalsCode(name, value);
                break;
        }
    }
    for (const item of replacements){
        const { replacementName, localName } = item;
        if (localName) {
            const { importName } = item;
            localsCode = localsCode.replace(new RegExp(replacementName, "g"), ()=>{
                if (options.modules.namedExport) {
                    return `" + ${importName}_NAMED___[${JSON.stringify((0, _camelcase.default)(localName))}] + "`;
                } else if (options.modules.exportOnlyLocals) {
                    return `" + ${importName}[${JSON.stringify(localName)}] + "`;
                }
                return `" + ${importName}.locals[${JSON.stringify(localName)}] + "`;
            });
        } else {
            localsCode = localsCode.replace(new RegExp(replacementName, "g"), ()=>`" + ${replacementName} + "`);
        }
    }
    if (options.modules.exportOnlyLocals) {
        code += options.modules.namedExport ? localsCode : `${options.esModule ? "export default" : "module.exports ="} {\n${localsCode}\n};\n`;
        return code;
    }
    if (localsCode) {
        code += options.modules.namedExport ? localsCode : `___CSS_LOADER_EXPORT___.locals = {\n${localsCode}\n};\n`;
    }
    code += `${options.esModule ? "export default" : "module.exports ="} ___CSS_LOADER_EXPORT___;\n`;
    return code;
}

//# sourceMappingURL=codegen.js.map