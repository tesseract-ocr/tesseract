"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getModuleBuildError", {
    enumerable: true,
    get: function() {
        return getModuleBuildError;
    }
});
const _fs = require("fs");
const _path = /*#__PURE__*/ _interop_require_wildcard(require("path"));
const _parseBabel = require("./parseBabel");
const _parseCss = require("./parseCss");
const _parseScss = require("./parseScss");
const _parseNotFoundError = require("./parseNotFoundError");
const _iserror = /*#__PURE__*/ _interop_require_default(require("../../../../lib/is-error"));
const _parseRSC = require("./parseRSC");
const _parseNextFontError = require("./parseNextFontError");
const _parseNextAppLoaderError = require("./parseNextAppLoaderError");
const _parseNextInvalidImportError = require("./parseNextInvalidImportError");
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
function getFileData(compilation, m) {
    var _compilation_compiler;
    let resolved;
    let ctx = ((_compilation_compiler = compilation.compiler) == null ? void 0 : _compilation_compiler.context) ?? null;
    if (ctx !== null && typeof m.resource === "string") {
        const res = _path.relative(ctx, m.resource).replace(/\\/g, _path.posix.sep);
        resolved = res.startsWith(".") ? res : `.${_path.posix.sep}${res}`;
    } else {
        const requestShortener = compilation.requestShortener;
        if (typeof (m == null ? void 0 : m.readableIdentifier) === "function") {
            resolved = m.readableIdentifier(requestShortener);
        } else {
            resolved = m.request ?? m.userRequest;
        }
    }
    if (resolved) {
        let content = null;
        try {
            content = (0, _fs.readFileSync)(ctx ? _path.resolve(ctx, resolved) : resolved, "utf8");
        } catch  {}
        return [
            resolved,
            content
        ];
    }
    return [
        "<unknown>",
        null
    ];
}
async function getModuleBuildError(compiler, compilation, input) {
    if (!(typeof input === "object" && ((input == null ? void 0 : input.name) === "ModuleBuildError" || (input == null ? void 0 : input.name) === "ModuleNotFoundError") && Boolean(input.module) && (0, _iserror.default)(input.error))) {
        return false;
    }
    const err = input.error;
    const [sourceFilename, sourceContent] = getFileData(compilation, input.module);
    const notFoundError = await (0, _parseNotFoundError.getNotFoundError)(compilation, input, sourceFilename, input.module);
    if (notFoundError !== false) {
        return notFoundError;
    }
    const imageError = await (0, _parseNotFoundError.getImageError)(compilation, input, err);
    if (imageError !== false) {
        return imageError;
    }
    const babel = (0, _parseBabel.getBabelError)(sourceFilename, err);
    if (babel !== false) {
        return babel;
    }
    const css = (0, _parseCss.getCssError)(sourceFilename, err);
    if (css !== false) {
        return css;
    }
    const scss = (0, _parseScss.getScssError)(sourceFilename, sourceContent, err);
    if (scss !== false) {
        return scss;
    }
    const rsc = (0, _parseRSC.getRscError)(sourceFilename, err, input.module, compilation, compiler);
    if (rsc !== false) {
        return rsc;
    }
    const nextFont = (0, _parseNextFontError.getNextFontError)(err, input.module);
    if (nextFont !== false) {
        return nextFont;
    }
    const nextAppLoader = (0, _parseNextAppLoaderError.getNextAppLoaderError)(err, input.module, compiler);
    if (nextAppLoader !== false) {
        return nextAppLoader;
    }
    const invalidImportError = (0, _parseNextInvalidImportError.getNextInvalidImportError)(err, input.module, compilation, compiler);
    if (invalidImportError !== false) {
        return invalidImportError;
    }
    return false;
}

//# sourceMappingURL=webpackModuleError.js.map