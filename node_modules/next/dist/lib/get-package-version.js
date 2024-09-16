"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getDependencies: null,
    getPackageVersion: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getDependencies: function() {
        return getDependencies;
    },
    getPackageVersion: function() {
        return getPackageVersion;
    }
});
const _fs = require("fs");
const _findup = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/find-up"));
const _json5 = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/json5"));
const _path = /*#__PURE__*/ _interop_require_wildcard(require("path"));
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
let cachedDeps;
function getDependencies({ cwd }) {
    if (cachedDeps) {
        return cachedDeps;
    }
    return cachedDeps = (async ()=>{
        const configurationPath = await (0, _findup.default)("package.json", {
            cwd
        });
        if (!configurationPath) {
            return {
                dependencies: {},
                devDependencies: {}
            };
        }
        const content = await _fs.promises.readFile(configurationPath, "utf-8");
        const packageJson = _json5.default.parse(content);
        const { dependencies = {}, devDependencies = {} } = packageJson || {};
        return {
            dependencies,
            devDependencies
        };
    })();
}
async function getPackageVersion({ cwd, name }) {
    const { dependencies, devDependencies } = await getDependencies({
        cwd
    });
    if (!(dependencies[name] || devDependencies[name])) {
        return null;
    }
    const cwd2 = cwd.endsWith(_path.posix.sep) || cwd.endsWith(_path.win32.sep) ? cwd : `${cwd}/`;
    try {
        const targetPath = require.resolve(`${name}/package.json`, {
            paths: [
                cwd2
            ]
        });
        const targetContent = await _fs.promises.readFile(targetPath, "utf-8");
        return _json5.default.parse(targetContent).version ?? null;
    } catch  {
        return null;
    }
}

//# sourceMappingURL=get-package-version.js.map