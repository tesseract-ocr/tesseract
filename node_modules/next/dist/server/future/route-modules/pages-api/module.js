"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    PagesAPIRouteModule: null,
    default: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    PagesAPIRouteModule: function() {
        return PagesAPIRouteModule;
    },
    default: function() {
        return _default;
    }
});
const _apiutils = require("../../../api-utils");
const _routemodule = require("../route-module");
const _apiresolver = require("../../../api-utils/node/api-resolver");
class PagesAPIRouteModule extends _routemodule.RouteModule {
    constructor(options){
        super(options);
        if (typeof options.userland.default !== "function") {
            throw new Error(`Page ${options.definition.page} does not export a default function.`);
        }
        this.apiResolverWrapped = (0, _apiutils.wrapApiHandler)(options.definition.page, _apiresolver.apiResolver);
    }
    /**
   *
   * @param req the incoming server request
   * @param res the outgoing server response
   * @param context the context for the render
   */ async render(req, res, context) {
        const { apiResolverWrapped } = this;
        await apiResolverWrapped(req, res, context.query, this.userland, {
            ...context.previewProps,
            revalidate: context.revalidate,
            trustHostHeader: context.trustHostHeader,
            allowedRevalidateHeaderKeys: context.allowedRevalidateHeaderKeys,
            hostname: context.hostname,
            multiZoneDraftMode: context.multiZoneDraftMode
        }, context.minimalMode, context.dev, context.page);
    }
}
const _default = PagesAPIRouteModule;

//# sourceMappingURL=module.js.map