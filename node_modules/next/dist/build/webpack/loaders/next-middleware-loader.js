"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    decodeMatchers: null,
    default: null,
    encodeMatchers: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    decodeMatchers: function() {
        return decodeMatchers;
    },
    default: function() {
        return middlewareLoader;
    },
    encodeMatchers: function() {
        return encodeMatchers;
    }
});
const _getmodulebuildinfo = require("./get-module-build-info");
const _constants = require("../../../lib/constants");
const _loadentrypoint = require("../../load-entrypoint");
function encodeMatchers(matchers) {
    return Buffer.from(JSON.stringify(matchers)).toString('base64');
}
function decodeMatchers(encodedMatchers) {
    return JSON.parse(Buffer.from(encodedMatchers, 'base64').toString());
}
async function middlewareLoader() {
    const { absolutePagePath, page, rootDir, matchers: encodedMatchers, preferredRegion, middlewareConfig: middlewareConfigBase64 } = this.getOptions();
    const matchers = encodedMatchers ? decodeMatchers(encodedMatchers) : undefined;
    const pagePath = this.utils.contextify(this.context || this.rootContext, absolutePagePath);
    const middlewareConfig = JSON.parse(Buffer.from(middlewareConfigBase64, 'base64').toString());
    const buildInfo = (0, _getmodulebuildinfo.getModuleBuildInfo)(this._module);
    buildInfo.nextEdgeMiddleware = {
        matchers,
        page: page.replace(new RegExp(`/${_constants.MIDDLEWARE_LOCATION_REGEXP}$`), '') || '/'
    };
    buildInfo.rootDir = rootDir;
    buildInfo.route = {
        page,
        absolutePagePath,
        preferredRegion,
        middlewareConfig
    };
    return await (0, _loadentrypoint.loadEntrypoint)('middleware', {
        VAR_USERLAND: pagePath,
        VAR_DEFINITION_PAGE: page
    });
}

//# sourceMappingURL=next-middleware-loader.js.map