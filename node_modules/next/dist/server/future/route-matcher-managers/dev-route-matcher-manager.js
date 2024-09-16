"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "DevRouteMatcherManager", {
    enumerable: true,
    get: function() {
        return DevRouteMatcherManager;
    }
});
const _routekind = require("../route-kind");
const _defaultroutematchermanager = require("./default-route-matcher-manager");
const _path = /*#__PURE__*/ _interop_require_default(require("../../../shared/lib/isomorphic/path"));
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../../../build/output/log"));
const _picocolors = require("../../../lib/picocolors");
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
class DevRouteMatcherManager extends _defaultroutematchermanager.DefaultRouteMatcherManager {
    constructor(production, ensurer, dir){
        super();
        this.production = production;
        this.ensurer = ensurer;
        this.dir = dir;
    }
    async test(pathname, options) {
        // Try to find a match within the developer routes.
        const match = await super.match(pathname, options);
        // Return if the match wasn't null. Unlike the implementation of `match`
        // which uses `matchAll` here, this does not call `ensure` on the match
        // found via the development matches.
        return match !== null;
    }
    validate(pathname, matcher, options) {
        const match = super.validate(pathname, matcher, options);
        // If a match was found, check to see if there were any conflicting app or
        // pages files.
        // TODO: maybe expand this to _any_ duplicated routes instead?
        if (match && matcher.duplicated && matcher.duplicated.some((duplicate)=>duplicate.definition.kind === _routekind.RouteKind.APP_PAGE || duplicate.definition.kind === _routekind.RouteKind.APP_ROUTE) && matcher.duplicated.some((duplicate)=>duplicate.definition.kind === _routekind.RouteKind.PAGES || duplicate.definition.kind === _routekind.RouteKind.PAGES_API)) {
            return null;
        }
        return match;
    }
    async *matchAll(pathname, options) {
        // Compile the development routes.
        // TODO: we may want to only run this during testing, users won't be fast enough to require this many dir scans
        await super.reload();
        // Iterate over the development matches to see if one of them match the
        // request path.
        for await (const development of super.matchAll(pathname, options)){
            // We're here, which means that we haven't seen this match yet, so we
            // should try to ensure it and recompile the production matcher.
            await this.ensurer.ensure(development, pathname);
            await this.production.reload();
            // Iterate over the production matches again, this time we should be able
            // to match it against the production matcher unless there's an error.
            for await (const production of this.production.matchAll(pathname, options)){
                yield production;
            }
        }
        // We tried direct matching against the pathname and against all the dynamic
        // paths, so there was no match.
        return null;
    }
    async reload() {
        // Compile the production routes again.
        await this.production.reload();
        // Compile the development routes.
        await super.reload();
        // Check for and warn of any duplicates.
        for (const [pathname, matchers] of Object.entries(this.matchers.duplicates)){
            // We only want to warn about matchers resolving to the same path if their
            // identities are different.
            const identity = matchers[0].identity;
            if (matchers.slice(1).some((matcher)=>matcher.identity !== identity)) {
                continue;
            }
            _log.warn(`Duplicate page detected. ${matchers.map((matcher)=>(0, _picocolors.cyan)(_path.default.relative(this.dir, matcher.definition.filename))).join(" and ")} resolve to ${(0, _picocolors.cyan)(pathname)}`);
        }
    }
}

//# sourceMappingURL=dev-route-matcher-manager.js.map