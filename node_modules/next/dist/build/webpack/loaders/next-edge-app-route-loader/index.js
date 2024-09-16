"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return _default;
    }
});
const _getmodulebuildinfo = require("../get-module-build-info");
const _stringifyrequest = require("../../stringify-request");
const _constants = require("../../../../lib/constants");
const _loadentrypoint = require("../../../load-entrypoint");
const EdgeAppRouteLoader = async function() {
    const { page, absolutePagePath, preferredRegion, appDirLoader: appDirLoaderBase64 = "", middlewareConfig: middlewareConfigBase64 = "" } = this.getOptions();
    const appDirLoader = Buffer.from(appDirLoaderBase64, "base64").toString();
    const middlewareConfig = JSON.parse(Buffer.from(middlewareConfigBase64, "base64").toString());
    // Ensure we only run this loader for as a module.
    if (!this._module) throw new Error("This loader is only usable as a module");
    const buildInfo = (0, _getmodulebuildinfo.getModuleBuildInfo)(this._module);
    buildInfo.nextEdgeSSR = {
        isServerComponent: false,
        page: page,
        isAppDir: true
    };
    buildInfo.route = {
        page,
        absolutePagePath,
        preferredRegion,
        middlewareConfig
    };
    const stringifiedPagePath = (0, _stringifyrequest.stringifyRequest)(this, absolutePagePath);
    const modulePath = `${appDirLoader}${stringifiedPagePath.substring(1, stringifiedPagePath.length - 1)}?${_constants.WEBPACK_RESOURCE_QUERIES.edgeSSREntry}`;
    return await (0, _loadentrypoint.loadEntrypoint)("edge-app-route", {
        VAR_USERLAND: modulePath
    });
};
const _default = EdgeAppRouteLoader;

//# sourceMappingURL=index.js.map