"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getEntryInfo: null,
    getInfo: null,
    getSource: null,
    getTs: null,
    getTypeChecker: null,
    init: null,
    isAppEntryFile: null,
    isDefaultFunctionExport: null,
    isInsideApp: null,
    isPageFile: null,
    isPositionInsideNode: null,
    log: null,
    removeStringQuotes: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getEntryInfo: function() {
        return getEntryInfo;
    },
    getInfo: function() {
        return getInfo;
    },
    getSource: function() {
        return getSource;
    },
    getTs: function() {
        return getTs;
    },
    getTypeChecker: function() {
        return getTypeChecker;
    },
    init: function() {
        return init;
    },
    isAppEntryFile: function() {
        return isAppEntryFile;
    },
    isDefaultFunctionExport: function() {
        return isDefaultFunctionExport;
    },
    isInsideApp: function() {
        return isInsideApp;
    },
    isPageFile: function() {
        return isPageFile;
    },
    isPositionInsideNode: function() {
        return isPositionInsideNode;
    },
    log: function() {
        return log;
    },
    removeStringQuotes: function() {
        return removeStringQuotes;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
let ts;
let info;
let appDirRegExp;
function log(message) {
    info.project.projectService.logger.info(message);
}
function init(opts) {
    ts = opts.ts;
    info = opts.info;
    const projectDir = info.project.getCurrentDirectory();
    appDirRegExp = new RegExp('^' + (projectDir + '(/src)?/app').replace(/[\\/]/g, '[\\/]'));
    log('Starting Next.js TypeScript plugin: ' + projectDir);
}
function getTs() {
    return ts;
}
function getInfo() {
    return info;
}
function getTypeChecker() {
    var _info_languageService_getProgram;
    return (_info_languageService_getProgram = info.languageService.getProgram()) == null ? void 0 : _info_languageService_getProgram.getTypeChecker();
}
function getSource(fileName) {
    var _info_languageService_getProgram;
    return (_info_languageService_getProgram = info.languageService.getProgram()) == null ? void 0 : _info_languageService_getProgram.getSourceFile(fileName);
}
function removeStringQuotes(str) {
    return str.replace(/^['"`]|['"`]$/g, '');
}
const isPositionInsideNode = (position, node)=>{
    const start = node.getFullStart();
    return start <= position && position <= node.getFullWidth() + start;
};
const isDefaultFunctionExport = (node)=>{
    if (ts.isFunctionDeclaration(node)) {
        let hasExportKeyword = false;
        let hasDefaultKeyword = false;
        if (node.modifiers) {
            for (const modifier of node.modifiers){
                if (modifier.kind === ts.SyntaxKind.ExportKeyword) {
                    hasExportKeyword = true;
                } else if (modifier.kind === ts.SyntaxKind.DefaultKeyword) {
                    hasDefaultKeyword = true;
                }
            }
        }
        // `export default function`
        if (hasExportKeyword && hasDefaultKeyword) {
            return true;
        }
    }
    return false;
};
const isInsideApp = (filePath)=>{
    return appDirRegExp.test(filePath);
};
const isAppEntryFile = (filePath)=>{
    return appDirRegExp.test(filePath) && /^(page|layout)\.(mjs|js|jsx|ts|tsx)$/.test(_path.default.basename(filePath));
};
const isPageFile = (filePath)=>{
    return appDirRegExp.test(filePath) && /^page\.(mjs|js|jsx|ts|tsx)$/.test(_path.default.basename(filePath));
};
function getEntryInfo(fileName, throwOnInvalidDirective) {
    const source = getSource(fileName);
    if (source) {
        let isDirective = true;
        let isClientEntry = false;
        let isServerEntry = false;
        ts.forEachChild(source, (node)=>{
            if (ts.isExpressionStatement(node) && ts.isStringLiteral(node.expression)) {
                if (node.expression.text === 'use client') {
                    if (isDirective) {
                        isClientEntry = true;
                    } else {
                        if (throwOnInvalidDirective) {
                            const e = {
                                messageText: 'The `"use client"` directive must be put at the top of the file.',
                                start: node.expression.getStart(),
                                length: node.expression.getWidth()
                            };
                            throw e;
                        }
                    }
                } else if (node.expression.text === 'use server') {
                    if (isDirective) {
                        isServerEntry = true;
                    } else {
                        if (throwOnInvalidDirective) {
                            const e = {
                                messageText: 'The `"use server"` directive must be put at the top of the file.',
                                start: node.expression.getStart(),
                                length: node.expression.getWidth()
                            };
                            throw e;
                        }
                    }
                }
                if (isClientEntry && isServerEntry) {
                    const e = {
                        messageText: 'Cannot use both "use client" and "use server" directives in the same file.',
                        start: node.expression.getStart(),
                        length: node.expression.getWidth()
                    };
                    throw e;
                }
            } else {
                isDirective = false;
            }
        });
        return {
            client: isClientEntry,
            server: isServerEntry
        };
    }
    return {
        client: false,
        server: false
    };
}

//# sourceMappingURL=utils.js.map