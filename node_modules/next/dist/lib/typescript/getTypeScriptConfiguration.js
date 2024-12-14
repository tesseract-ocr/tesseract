"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getTypeScriptConfiguration", {
    enumerable: true,
    get: function() {
        return getTypeScriptConfiguration;
    }
});
const _picocolors = require("../picocolors");
const _os = /*#__PURE__*/ _interop_require_default(require("os"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _fatalerror = require("../fatal-error");
const _iserror = /*#__PURE__*/ _interop_require_default(require("../is-error"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function getTypeScriptConfiguration(ts, tsConfigPath, metaOnly) {
    try {
        var _result_errors;
        const formatDiagnosticsHost = {
            getCanonicalFileName: (fileName)=>fileName,
            getCurrentDirectory: ts.sys.getCurrentDirectory,
            getNewLine: ()=>_os.default.EOL
        };
        const { config, error } = ts.readConfigFile(tsConfigPath, ts.sys.readFile);
        if (error) {
            throw new _fatalerror.FatalError(ts.formatDiagnostic(error, formatDiagnosticsHost));
        }
        let configToParse = config;
        const result = ts.parseJsonConfigFileContent(configToParse, // When only interested in meta info,
        // avoid enumerating all files (for performance reasons)
        metaOnly ? {
            ...ts.sys,
            readDirectory (_path, extensions, _excludes, _includes, _depth) {
                return [
                    extensions ? `file${extensions[0]}` : `file.ts`
                ];
            }
        } : ts.sys, _path.default.dirname(tsConfigPath));
        if (result.errors) {
            result.errors = result.errors.filter(({ code })=>// No inputs were found in config file
                code !== 18003);
        }
        if ((_result_errors = result.errors) == null ? void 0 : _result_errors.length) {
            throw new _fatalerror.FatalError(ts.formatDiagnostic(result.errors[0], formatDiagnosticsHost));
        }
        return result;
    } catch (err) {
        if ((0, _iserror.default)(err) && err.name === 'SyntaxError') {
            const reason = '\n' + (err.message ?? '');
            throw new _fatalerror.FatalError((0, _picocolors.bold)('Could not parse' + (0, _picocolors.cyan)('tsconfig.json') + '.' + ' Please make sure it contains syntactically correct JSON.') + reason);
        }
        throw err;
    }
}

//# sourceMappingURL=getTypeScriptConfiguration.js.map