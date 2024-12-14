"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getStartServerInfo: null,
    logStartInfo: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getStartServerInfo: function() {
        return getStartServerInfo;
    },
    logStartInfo: function() {
        return logStartInfo;
    }
});
const _env = require("@next/env");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../../build/output/log"));
const _picocolors = require("../../lib/picocolors");
const _constants = require("../../shared/lib/constants");
const _config = /*#__PURE__*/ _interop_require_wildcard(require("../config"));
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
function logStartInfo({ networkUrl, appUrl, envInfo, expFeatureInfo, maxExperimentalFeatures = Infinity }) {
    _log.bootstrap(`${(0, _picocolors.bold)((0, _picocolors.purple)(`${_log.prefixes.ready} Next.js ${"15.1.0"}`))}${process.env.TURBOPACK ? ' (Turbopack)' : ''}`);
    if (appUrl) {
        _log.bootstrap(`- Local:        ${appUrl}`);
    }
    if (networkUrl) {
        _log.bootstrap(`- Network:      ${networkUrl}`);
    }
    if (envInfo == null ? void 0 : envInfo.length) _log.bootstrap(`- Environments: ${envInfo.join(', ')}`);
    if (expFeatureInfo == null ? void 0 : expFeatureInfo.length) {
        _log.bootstrap(`- Experiments (use with caution):`);
        // only show a maximum number of flags
        for (const exp of expFeatureInfo.slice(0, maxExperimentalFeatures)){
            _log.bootstrap(`  · ${exp}`);
        }
        /* indicate if there are more than the maximum shown no. flags */ if (expFeatureInfo.length > maxExperimentalFeatures) {
            _log.bootstrap(`  · ...`);
        }
    }
    // New line after the bootstrap info
    _log.info('');
}
async function getStartServerInfo(dir, dev) {
    let expFeatureInfo = [];
    await (0, _config.default)(dev ? _constants.PHASE_DEVELOPMENT_SERVER : _constants.PHASE_PRODUCTION_BUILD, dir, {
        onLoadUserConfig (userConfig) {
            const userNextConfigExperimental = (0, _config.getEnabledExperimentalFeatures)(userConfig.experimental);
            expFeatureInfo = userNextConfigExperimental.sort((a, b)=>a.length - b.length);
        }
    });
    // we need to reset env if we are going to create
    // the worker process with the esm loader so that the
    // initial env state is correct
    let envInfo = [];
    const { loadedEnvFiles } = (0, _env.loadEnvConfig)(dir, true, console, false);
    if (loadedEnvFiles.length > 0) {
        envInfo = loadedEnvFiles.map((f)=>f.path);
    }
    return {
        envInfo,
        expFeatureInfo
    };
}

//# sourceMappingURL=app-info-log.js.map