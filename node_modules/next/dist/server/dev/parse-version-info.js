"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "parseVersionInfo", {
    enumerable: true,
    get: function() {
        return parseVersionInfo;
    }
});
const _semver = /*#__PURE__*/ _interop_require_wildcard(require("next/dist/compiled/semver"));
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
function parseVersionInfo(o) {
    const latest = _semver.parse(o.latest);
    const canary = _semver.parse(o.canary);
    const installedParsed = _semver.parse(o.installed);
    const installed = o.installed;
    if (installedParsed && latest && canary) {
        if (installedParsed.major < latest.major) {
            // Old major version
            return {
                staleness: 'stale-major',
                expected: latest.raw,
                installed
            };
        } else if (installedParsed.prerelease[0] === 'canary' && _semver.lt(installedParsed, canary)) {
            // Matching major, but old canary
            return {
                staleness: 'stale-prerelease',
                expected: canary.raw,
                installed
            };
        } else if (!installedParsed.prerelease.length && _semver.lt(installedParsed, latest)) {
            // Stable, but not the latest
            if (installedParsed.minor === latest.minor) {
                // Same major and minor, but not the latest patch
                return {
                    staleness: 'stale-patch',
                    expected: latest.raw,
                    installed
                };
            }
            return {
                staleness: 'stale-minor',
                expected: latest.raw,
                installed
            };
        } else if (_semver.gt(installedParsed, latest) && installedParsed.version !== canary.version) {
            // Newer major version
            return {
                staleness: 'newer-than-npm',
                installed
            };
        } else {
            // Latest and greatest
            return {
                staleness: 'fresh',
                installed
            };
        }
    }
    return {
        installed: (installedParsed == null ? void 0 : installedParsed.raw) ?? '0.0.0',
        staleness: 'unknown'
    };
}

//# sourceMappingURL=parse-version-info.js.map