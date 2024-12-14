import { webpack, sources } from 'next/dist/compiled/webpack/webpack';
import { APP_BUILD_MANIFEST, CLIENT_STATIC_FILES_RUNTIME_MAIN_APP, SYSTEM_ENTRYPOINTS } from '../../../shared/lib/constants';
import { getEntrypointFiles } from './build-manifest-plugin';
import getAppRouteFromEntrypoint from '../../../server/get-app-route-from-entrypoint';
const PLUGIN_NAME = 'AppBuildManifestPlugin';
export class AppBuildManifestPlugin {
    constructor(options){
        this.dev = options.dev;
    }
    apply(compiler) {
        compiler.hooks.compilation.tap(PLUGIN_NAME, (compilation, { normalModuleFactory })=>{
            compilation.dependencyFactories.set(webpack.dependencies.ModuleDependency, normalModuleFactory);
            compilation.dependencyTemplates.set(webpack.dependencies.ModuleDependency, new webpack.dependencies.NullDependency.Template());
        });
        compiler.hooks.make.tap(PLUGIN_NAME, (compilation)=>{
            compilation.hooks.processAssets.tap({
                name: PLUGIN_NAME,
                stage: webpack.Compilation.PROCESS_ASSETS_STAGE_ADDITIONS
            }, (assets)=>this.createAsset(assets, compilation));
        });
    }
    createAsset(assets, compilation) {
        const manifest = {
            pages: {}
        };
        const mainFiles = new Set(getEntrypointFiles(compilation.entrypoints.get(CLIENT_STATIC_FILES_RUNTIME_MAIN_APP)));
        for (const entrypoint of compilation.entrypoints.values()){
            if (!entrypoint.name) {
                continue;
            }
            if (SYSTEM_ENTRYPOINTS.has(entrypoint.name)) {
                continue;
            }
            const pagePath = getAppRouteFromEntrypoint(entrypoint.name);
            if (!pagePath) {
                continue;
            }
            const filesForPage = getEntrypointFiles(entrypoint);
            manifest.pages[pagePath] = [
                ...new Set([
                    ...mainFiles,
                    ...filesForPage
                ])
            ];
        }
        const json = JSON.stringify(manifest, null, 2);
        assets[APP_BUILD_MANIFEST] = new sources.RawSource(json);
    }
}

//# sourceMappingURL=app-build-manifest-plugin.js.map