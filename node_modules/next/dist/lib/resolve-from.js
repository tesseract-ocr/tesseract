// source: https://github.com/sindresorhus/resolve-from
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "resolveFrom", {
    enumerable: true,
    get: function() {
        return resolveFrom;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _iserror = /*#__PURE__*/ _interop_require_default(require("./is-error"));
const _realpath = require("./realpath");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const Module = require('module');
const resolveFrom = (fromDirectory, moduleId, silent)=>{
    if (typeof fromDirectory !== 'string') {
        throw new TypeError(`Expected \`fromDir\` to be of type \`string\`, got \`${typeof fromDirectory}\``);
    }
    if (typeof moduleId !== 'string') {
        throw new TypeError(`Expected \`moduleId\` to be of type \`string\`, got \`${typeof moduleId}\``);
    }
    try {
        fromDirectory = (0, _realpath.realpathSync)(fromDirectory);
    } catch (error) {
        if ((0, _iserror.default)(error) && error.code === 'ENOENT') {
            fromDirectory = _path.default.resolve(fromDirectory);
        } else if (silent) {
            return;
        } else {
            throw error;
        }
    }
    const fromFile = _path.default.join(fromDirectory, 'noop.js');
    const resolveFileName = ()=>Module._resolveFilename(moduleId, {
            id: fromFile,
            filename: fromFile,
            paths: Module._nodeModulePaths(fromDirectory)
        });
    if (silent) {
        try {
            return resolveFileName();
        } catch (error) {
            return;
        }
    }
    return resolveFileName();
};

//# sourceMappingURL=resolve-from.js.map