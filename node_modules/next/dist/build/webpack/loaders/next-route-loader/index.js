"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    default: null,
    getRouteLoaderEntry: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    default: function() {
        return _default;
    },
    getRouteLoaderEntry: function() {
        return getRouteLoaderEntry;
    }
});
const _querystring = require("querystring");
const _getmodulebuildinfo = require("../get-module-build-info");
const _routekind = require("../../../../server/route-kind");
const _normalizepagepath = require("../../../../shared/lib/page-path/normalize-page-path");
const _utils = require("../utils");
const _utils1 = require("../../../utils");
const _loadentrypoint = require("../../../load-entrypoint");
function getRouteLoaderEntry(options) {
    switch(options.kind){
        case _routekind.RouteKind.PAGES:
            {
                const query = {
                    kind: options.kind,
                    page: options.page,
                    preferredRegion: options.preferredRegion,
                    absolutePagePath: options.absolutePagePath,
                    // These are the path references to the internal components that may be
                    // overridden by userland components.
                    absoluteAppPath: options.pages['/_app'],
                    absoluteDocumentPath: options.pages['/_document'],
                    middlewareConfigBase64: (0, _utils.encodeToBase64)(options.middlewareConfig)
                };
                return `next-route-loader?${(0, _querystring.stringify)(query)}!`;
            }
        case _routekind.RouteKind.PAGES_API:
            {
                const query = {
                    kind: options.kind,
                    page: options.page,
                    preferredRegion: options.preferredRegion,
                    absolutePagePath: options.absolutePagePath,
                    middlewareConfigBase64: (0, _utils.encodeToBase64)(options.middlewareConfig)
                };
                return `next-route-loader?${(0, _querystring.stringify)(query)}!`;
            }
        default:
            {
                throw new Error('Invariant: Unexpected route kind');
            }
    }
}
const loadPages = async ({ page, absolutePagePath, absoluteDocumentPath, absoluteAppPath, preferredRegion, middlewareConfigBase64 }, buildInfo)=>{
    const middlewareConfig = (0, _utils.decodeFromBase64)(middlewareConfigBase64);
    // Attach build info to the module.
    buildInfo.route = {
        page,
        absolutePagePath,
        preferredRegion,
        middlewareConfig
    };
    let file = await (0, _loadentrypoint.loadEntrypoint)('pages', {
        VAR_USERLAND: absolutePagePath,
        VAR_MODULE_DOCUMENT: absoluteDocumentPath,
        VAR_MODULE_APP: absoluteAppPath,
        VAR_DEFINITION_PAGE: (0, _normalizepagepath.normalizePagePath)(page),
        VAR_DEFINITION_PATHNAME: page
    });
    if ((0, _utils1.isInstrumentationHookFile)(page)) {
        // When we're building the instrumentation page (only when the
        // instrumentation file conflicts with a page also labeled
        // /instrumentation) hoist the `register` method.
        file += '\nexport const register = hoist(userland, "register")';
    }
    return file;
};
const loadPagesAPI = async ({ page, absolutePagePath, preferredRegion, middlewareConfigBase64 }, buildInfo)=>{
    const middlewareConfig = (0, _utils.decodeFromBase64)(middlewareConfigBase64);
    // Attach build info to the module.
    buildInfo.route = {
        page,
        absolutePagePath,
        preferredRegion,
        middlewareConfig
    };
    return await (0, _loadentrypoint.loadEntrypoint)('pages-api', {
        VAR_USERLAND: absolutePagePath,
        VAR_DEFINITION_PAGE: (0, _normalizepagepath.normalizePagePath)(page),
        VAR_DEFINITION_PATHNAME: page
    });
};
/**
 * Handles the `next-route-loader` options.
 * @returns the loader definition function
 */ const loader = async function() {
    if (!this._module) {
        throw new Error('Invariant: expected this to reference a module');
    }
    const buildInfo = (0, _getmodulebuildinfo.getModuleBuildInfo)(this._module);
    const opts = this.getOptions();
    switch(opts.kind){
        case _routekind.RouteKind.PAGES:
            {
                return await loadPages(opts, buildInfo);
            }
        case _routekind.RouteKind.PAGES_API:
            {
                return await loadPagesAPI(opts, buildInfo);
            }
        default:
            {
                throw new Error('Invariant: Unexpected route kind');
            }
    }
};
const _default = loader;

//# sourceMappingURL=index.js.map