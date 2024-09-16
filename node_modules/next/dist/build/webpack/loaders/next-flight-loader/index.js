"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    default: null,
    getAssumedSourceType: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    default: function() {
        return transformSource;
    },
    getAssumedSourceType: function() {
        return getAssumedSourceType;
    }
});
const _constants = require("../../../../lib/constants");
const _constants1 = require("../../../../shared/lib/constants");
const _warnonce = require("../../../../shared/lib/utils/warn-once");
const _getpagestaticinfo = require("../../../analysis/get-page-static-info");
const _utils = require("../../utils");
const _getmodulebuildinfo = require("../get-module-build-info");
const noopHeadPath = require.resolve("next/dist/client/components/noop-head");
// For edge runtime it will be aliased to esm version by webpack
const MODULE_PROXY_PATH = "next/dist/build/webpack/loaders/next-flight-loader/module-proxy";
function getAssumedSourceType(mod, sourceType) {
    var _buildInfo_rsc, _buildInfo_rsc1;
    const buildInfo = (0, _getmodulebuildinfo.getModuleBuildInfo)(mod);
    const detectedClientEntryType = buildInfo == null ? void 0 : (_buildInfo_rsc = buildInfo.rsc) == null ? void 0 : _buildInfo_rsc.clientEntryType;
    const clientRefs = (buildInfo == null ? void 0 : (_buildInfo_rsc1 = buildInfo.rsc) == null ? void 0 : _buildInfo_rsc1.clientRefs) || [];
    // It's tricky to detect the type of a client boundary, but we should always
    // use the `module` type when we can, to support `export *` and `export from`
    // syntax in other modules that import this client boundary.
    let assumedSourceType = sourceType;
    if (assumedSourceType === "auto" && detectedClientEntryType === "auto") {
        if (clientRefs.length === 0 || clientRefs.length === 1 && clientRefs[0] === "") {
            // If there's zero export detected in the client boundary, and it's the
            // `auto` type, we can safely assume it's a CJS module because it doesn't
            // have ESM exports.
            assumedSourceType = "commonjs";
        } else if (!clientRefs.includes("*")) {
            // Otherwise, we assume it's an ESM module.
            assumedSourceType = "module";
        }
    }
    return assumedSourceType;
}
function transformSource(source, sourceMap) {
    var _this__module_matchResource, _this__module, _buildInfo_rsc, _buildInfo_rsc1;
    // Avoid buffer to be consumed
    if (typeof source !== "string") {
        throw new Error("Expected source to have been transformed to a string.");
    }
    // Assign the RSC meta information to buildInfo.
    // Exclude next internal files which are not marked as client files
    const buildInfo = (0, _getmodulebuildinfo.getModuleBuildInfo)(this._module);
    buildInfo.rsc = (0, _getpagestaticinfo.getRSCModuleInformation)(source, true);
    // Resource key is the unique identifier for the resource. When RSC renders
    // a client module, that key is used to identify that module across all compiler
    // layers.
    //
    // Usually it's the module's file path + the export name (e.g. `foo.js#bar`).
    // But with Barrel Optimizations, one file can be splitted into multiple modules,
    // so when you import `foo.js#bar` and `foo.js#baz`, they are actually different
    // "foo.js" being created by the Barrel Loader (one only exports `bar`, the other
    // only exports `baz`).
    //
    // Because of that, we must add another query param to the resource key to
    // differentiate them.
    let resourceKey = this.resourcePath;
    if ((_this__module = this._module) == null ? void 0 : (_this__module_matchResource = _this__module.matchResource) == null ? void 0 : _this__module_matchResource.startsWith(_constants1.BARREL_OPTIMIZATION_PREFIX)) {
        resourceKey = (0, _utils.formatBarrelOptimizedResource)(resourceKey, this._module.matchResource);
    }
    // A client boundary.
    if (((_buildInfo_rsc = buildInfo.rsc) == null ? void 0 : _buildInfo_rsc.type) === _constants1.RSC_MODULE_TYPES.client) {
        var _this__module_parser, _this__module1;
        const assumedSourceType = getAssumedSourceType(this._module, (_this__module1 = this._module) == null ? void 0 : (_this__module_parser = _this__module1.parser) == null ? void 0 : _this__module_parser.sourceType);
        const clientRefs = buildInfo.rsc.clientRefs;
        if (assumedSourceType === "module") {
            if (clientRefs.includes("*")) {
                this.callback(new Error(`It's currently unsupported to use "export *" in a client boundary. Please use named exports instead.`));
                return;
            }
            let esmSource = `\
import { createProxy } from "${MODULE_PROXY_PATH}"
`;
            let cnt = 0;
            for (const ref of clientRefs){
                if (ref === "") {
                    esmSource += `\nexports[''] = createProxy(String.raw\`${resourceKey}#\`);`;
                } else if (ref === "default") {
                    esmSource += `\
export default createProxy(String.raw\`${resourceKey}#default\`);
`;
                } else {
                    esmSource += `
const e${cnt} = createProxy(String.raw\`${resourceKey}#${ref}\`);
export { e${cnt++} as ${ref} };`;
                }
            }
            this.callback(null, esmSource, sourceMap);
            return;
        }
    }
    if (((_buildInfo_rsc1 = buildInfo.rsc) == null ? void 0 : _buildInfo_rsc1.type) !== _constants1.RSC_MODULE_TYPES.client) {
        if (noopHeadPath === this.resourcePath) {
            (0, _warnonce.warnOnce)(`Warning: You're using \`next/head\` inside the \`app\` directory, please migrate to the Metadata API. See https://nextjs.org/docs/app/building-your-application/upgrading/app-router-migration#step-3-migrating-nexthead for more details.`);
        }
    }
    const replacedSource = source.replace(_constants.RSC_MOD_REF_PROXY_ALIAS, MODULE_PROXY_PATH);
    this.callback(null, replacedSource, sourceMap);
}

//# sourceMappingURL=index.js.map