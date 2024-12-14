"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getCacheDirectory", {
    enumerable: true,
    get: function() {
        return getCacheDirectory;
    }
});
const _os = /*#__PURE__*/ _interop_require_default(require("os"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _fs = /*#__PURE__*/ _interop_require_default(require("fs"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function getCacheDirectory(fileDirectory, envPath) {
    let result;
    if (envPath) {
        result = envPath;
    } else {
        let systemCacheDirectory;
        if (process.platform === 'linux') {
            systemCacheDirectory = process.env.XDG_CACHE_HOME || _path.default.join(_os.default.homedir(), '.cache');
        } else if (process.platform === 'darwin') {
            systemCacheDirectory = _path.default.join(_os.default.homedir(), 'Library', 'Caches');
        } else if (process.platform === 'win32') {
            systemCacheDirectory = process.env.LOCALAPPDATA || _path.default.join(_os.default.homedir(), 'AppData', 'Local');
        } else {
            /// Attempt to use generic tmp location for un-handled platform
            if (!systemCacheDirectory) {
                for (const dir of [
                    _path.default.join(_os.default.homedir(), '.cache'),
                    _path.default.join(_os.default.tmpdir())
                ]){
                    if (_fs.default.existsSync(dir)) {
                        systemCacheDirectory = dir;
                        break;
                    }
                }
            }
            if (!systemCacheDirectory) {
                console.error(new Error('Unsupported platform: ' + process.platform));
                process.exit(0);
            }
        }
        result = _path.default.join(systemCacheDirectory, fileDirectory);
    }
    if (!_path.default.isAbsolute(result)) {
        // It is important to resolve to the absolute path:
        //   - for unzipping to work correctly;
        //   - so that registry directory matches between installation and execution.
        // INIT_CWD points to the root of `npm/yarn install` and is probably what
        // the user meant when typing the relative path.
        result = _path.default.resolve(process.env['INIT_CWD'] || process.cwd(), result);
    }
    return result;
}

//# sourceMappingURL=get-cache-directory.js.map