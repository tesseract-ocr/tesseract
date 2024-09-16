/**
 * This webpack resolver is largely based on TypeScript's "paths" handling
 * The TypeScript license can be found here:
 * https://github.com/microsoft/TypeScript/blob/214df64e287804577afa1fea0184c18c40f7d1ca/LICENSE.txt
 */ "use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    JsConfigPathsPlugin: null,
    findBestPatternMatch: null,
    hasZeroOrOneAsteriskCharacter: null,
    isString: null,
    matchPatternOrExact: null,
    matchedText: null,
    pathIsRelative: null,
    patternText: null,
    tryParsePattern: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    JsConfigPathsPlugin: function() {
        return JsConfigPathsPlugin;
    },
    findBestPatternMatch: function() {
        return findBestPatternMatch;
    },
    hasZeroOrOneAsteriskCharacter: function() {
        return hasZeroOrOneAsteriskCharacter;
    },
    isString: function() {
        return isString;
    },
    matchPatternOrExact: function() {
        return matchPatternOrExact;
    },
    matchedText: function() {
        return matchedText;
    },
    pathIsRelative: function() {
        return pathIsRelative;
    },
    patternText: function() {
        return patternText;
    },
    tryParsePattern: function() {
        return tryParsePattern;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _debug = require("next/dist/compiled/debug");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const log = (0, _debug.debug)("next:jsconfig-paths-plugin");
const asterisk = 0x2a;
function hasZeroOrOneAsteriskCharacter(str) {
    let seenAsterisk = false;
    for(let i = 0; i < str.length; i++){
        if (str.charCodeAt(i) === asterisk) {
            if (!seenAsterisk) {
                seenAsterisk = true;
            } else {
                // have already seen asterisk
                return false;
            }
        }
    }
    return true;
}
function pathIsRelative(testPath) {
    return /^\.\.?($|[\\/])/.test(testPath);
}
function tryParsePattern(pattern) {
    // This should be verified outside of here and a proper error thrown.
    const indexOfStar = pattern.indexOf("*");
    return indexOfStar === -1 ? undefined : {
        prefix: pattern.slice(0, indexOfStar),
        suffix: pattern.slice(indexOfStar + 1)
    };
}
function isPatternMatch({ prefix, suffix }, candidate) {
    return candidate.length >= prefix.length + suffix.length && candidate.startsWith(prefix) && candidate.endsWith(suffix);
}
function findBestPatternMatch(values, getPattern, candidate) {
    let matchedValue;
    // use length of prefix as betterness criteria
    let longestMatchPrefixLength = -1;
    for (const v of values){
        const pattern = getPattern(v);
        if (isPatternMatch(pattern, candidate) && pattern.prefix.length > longestMatchPrefixLength) {
            longestMatchPrefixLength = pattern.prefix.length;
            matchedValue = v;
        }
    }
    return matchedValue;
}
function matchPatternOrExact(patternStrings, candidate) {
    const patterns = [];
    for (const patternString of patternStrings){
        if (!hasZeroOrOneAsteriskCharacter(patternString)) continue;
        const pattern = tryParsePattern(patternString);
        if (pattern) {
            patterns.push(pattern);
        } else if (patternString === candidate) {
            // pattern was matched as is - no need to search further
            return patternString;
        }
    }
    return findBestPatternMatch(patterns, (_)=>_, candidate);
}
function isString(text) {
    return typeof text === "string";
}
function matchedText(pattern, candidate) {
    return candidate.substring(pattern.prefix.length, candidate.length - pattern.suffix.length);
}
function patternText({ prefix, suffix }) {
    return `${prefix}*${suffix}`;
}
/**
 * Calls the iterator function for each entry of the array
 * until the first result or error is reached
 */ function forEachBail(array, iterator, callback) {
    if (array.length === 0) return callback();
    let i = 0;
    const next = ()=>{
        let loop = undefined;
        iterator(array[i++], (err, result)=>{
            if (err || result !== undefined || i >= array.length) {
                return callback(err, result);
            }
            if (loop === false) while(next());
            loop = true;
        });
        if (!loop) loop = false;
        return loop;
    };
    while(next());
}
const NODE_MODULES_REGEX = /node_modules/;
class JsConfigPathsPlugin {
    constructor(paths, resolvedBaseUrl){
        this.paths = paths;
        this.resolvedBaseUrl = resolvedBaseUrl;
        this.jsConfigPlugin = true;
        log("tsconfig.json or jsconfig.json paths: %O", paths);
        log("resolved baseUrl: %s", resolvedBaseUrl);
    }
    apply(resolver) {
        const target = resolver.ensureHook("resolve");
        resolver.getHook("described-resolve").tapAsync("JsConfigPathsPlugin", (request, resolveContext, callback)=>{
            const resolvedBaseUrl = this.resolvedBaseUrl;
            if (resolvedBaseUrl === undefined) {
                return callback();
            }
            const paths = this.paths;
            const pathsKeys = Object.keys(paths);
            // If no aliases are added bail out
            if (pathsKeys.length === 0) {
                log("paths are empty, bailing out");
                return callback();
            }
            const moduleName = request.request;
            // Exclude node_modules from paths support (speeds up resolving)
            if (request.path.match(NODE_MODULES_REGEX)) {
                log("skipping request as it is inside node_modules %s", moduleName);
                return callback();
            }
            if (_path.default.posix.isAbsolute(moduleName) || process.platform === "win32" && _path.default.win32.isAbsolute(moduleName)) {
                log("skipping request as it is an absolute path %s", moduleName);
                return callback();
            }
            if (pathIsRelative(moduleName)) {
                log("skipping request as it is a relative path %s", moduleName);
                return callback();
            }
            // log('starting to resolve request %s', moduleName)
            // If the module name does not match any of the patterns in `paths` we hand off resolving to webpack
            const matchedPattern = matchPatternOrExact(pathsKeys, moduleName);
            if (!matchedPattern) {
                log("moduleName did not match any paths pattern %s", moduleName);
                return callback();
            }
            const matchedStar = isString(matchedPattern) ? undefined : matchedText(matchedPattern, moduleName);
            const matchedPatternText = isString(matchedPattern) ? matchedPattern : patternText(matchedPattern);
            let triedPaths = [];
            forEachBail(paths[matchedPatternText], (subst, pathCallback)=>{
                const curPath = matchedStar ? subst.replace("*", matchedStar) : subst;
                // Ensure .d.ts is not matched
                if (curPath.endsWith(".d.ts")) {
                    // try next path candidate
                    return pathCallback();
                }
                const candidate = _path.default.join(resolvedBaseUrl.baseUrl, curPath);
                const obj = Object.assign({}, request, {
                    request: candidate
                });
                resolver.doResolve(target, obj, `Aliased with tsconfig.json or jsconfig.json ${matchedPatternText} to ${candidate}`, resolveContext, (resolverErr, resolverResult)=>{
                    if (resolverErr || resolverResult === undefined) {
                        triedPaths.push(candidate);
                        // try next path candidate
                        return pathCallback();
                    }
                    return pathCallback(resolverErr, resolverResult);
                });
            }, callback);
        });
    }
}

//# sourceMappingURL=jsconfig-paths-plugin.js.map