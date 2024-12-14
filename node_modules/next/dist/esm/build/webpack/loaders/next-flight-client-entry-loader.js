import { BARREL_OPTIMIZATION_PREFIX, RSC_MODULE_TYPES } from '../../../shared/lib/constants';
import { getModuleBuildInfo } from './get-module-build-info';
import { regexCSS } from './utils';
export default function transformSource() {
    let { modules, server } = this.getOptions();
    const isServer = server === 'true';
    if (!Array.isArray(modules)) {
        modules = modules ? [
            modules
        ] : [];
    }
    const code = modules.map((x)=>JSON.parse(x))// Filter out CSS files in the SSR compilation
    .filter(({ request })=>isServer ? !regexCSS.test(request) : true).map(({ request, ids })=>{
        const importPath = JSON.stringify(request.startsWith(BARREL_OPTIMIZATION_PREFIX) ? request.replace(':', '!=!') : request);
        // When we cannot determine the export names, we use eager mode to include the whole module.
        // Otherwise, we use eager mode with webpackExports to only include the necessary exports.
        // If we have '*' in the ids, we include all the imports
        if (ids.length === 0 || ids.includes('*')) {
            return `import(/* webpackMode: "eager" */ ${importPath});\n`;
        } else {
            return `import(/* webpackMode: "eager", webpackExports: ${JSON.stringify(ids)} */ ${importPath});\n`;
        }
    }).join(';\n');
    const buildInfo = getModuleBuildInfo(this._module);
    buildInfo.rsc = {
        type: RSC_MODULE_TYPES.client
    };
    return code;
}

//# sourceMappingURL=next-flight-client-entry-loader.js.map