"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "AppBuildManifestPlugin", {
    enumerable: true,
    get: function() {
        return AppBuildManifestPlugin;
    }
});
const _webpack = require("next/dist/compiled/webpack/webpack");
const _constants = require("../../../shared/lib/constants");
const _buildmanifestplugin = require("./build-manifest-plugin");
const _getapproutefromentrypoint = /*#__PURE__*/ _interop_require_default(require("../../../server/get-app-route-from-entrypoint"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const PLUGIN_NAME = "AppBuildManifestPlugin";
class AppBuildManifestPlugin {
    constructor(options){
        this.dev = options.dev;
    }
    apply(compiler) {
        compiler.hooks.compilation.tap(PLUGIN_NAME, (compilation, { normalModuleFactory })=>{
            compilation.dependencyFactories.set(_webpack.webpack.dependencies.ModuleDependency, normalModuleFactory);
            compilation.dependencyTemplates.set(_webpack.webpack.dependencies.ModuleDependency, new _webpack.webpack.dependencies.NullDependency.Template());
        });
        compiler.hooks.make.tap(PLUGIN_NAME, (compilation)=>{
            compilation.hooks.processAssets.tap({
                name: PLUGIN_NAME,
                stage: _webpack.webpack.Compilation.PROCESS_ASSETS_STAGE_ADDITIONS
            }, (assets)=>this.createAsset(assets, compilation));
        });
    }
    createAsset(assets, compilation) {
        const manifest = {
            pages: {}
        };
        const mainFiles = new Set((0, _buildmanifestplugin.getEntrypointFiles)(compilation.entrypoints.get(_constants.CLIENT_STATIC_FILES_RUNTIME_MAIN_APP)));
        for (const entrypoint of compilation.entrypoints.values()){
            if (!entrypoint.name) {
                continue;
            }
            if (_constants.SYSTEM_ENTRYPOINTS.has(entrypoint.name)) {
                continue;
            }
            const pagePath = (0, _getapproutefromentrypoint.default)(entrypoint.name);
            if (!pagePath) {
                continue;
            }
            const filesForPage = (0, _buildmanifestplugin.getEntrypointFiles)(entrypoint);
            manifest.pages[pagePath] = [
                ...new Set([
                    ...mainFiles,
                    ...filesForPage
                ])
            ];
        }
        const json = JSON.stringify(manifest, null, 2);
        assets[_constants.APP_BUILD_MANIFEST] = new _webpack.sources.RawSource(json);
    }
}

//# sourceMappingURL=app-build-manifest-plugin.js.map