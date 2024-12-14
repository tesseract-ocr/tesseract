"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "eventSwcPlugins", {
    enumerable: true,
    get: function() {
        return eventSwcPlugins;
    }
});
const _findup = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/find-up"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _fs = /*#__PURE__*/ _interop_require_default(require("fs"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const EVENT_SWC_PLUGIN_PRESENT = 'NEXT_SWC_PLUGIN_DETECTED';
async function eventSwcPlugins(dir, config) {
    try {
        var _config_experimental_swcPlugins, _config_experimental;
        const packageJsonPath = await (0, _findup.default)('package.json', {
            cwd: dir
        });
        if (!packageJsonPath) {
            return [];
        }
        const { dependencies = {}, devDependencies = {} } = require(packageJsonPath);
        const deps = {
            ...devDependencies,
            ...dependencies
        };
        const swcPluginPackages = ((_config_experimental = config.experimental) == null ? void 0 : (_config_experimental_swcPlugins = _config_experimental.swcPlugins) == null ? void 0 : _config_experimental_swcPlugins.map(([name, _])=>name)) ?? [];
        return swcPluginPackages.map((plugin)=>{
            // swc plugins can be non-npm pkgs with absolute path doesn't have version
            const version = deps[plugin] ?? undefined;
            let pluginName = plugin;
            if (_fs.default.existsSync(pluginName)) {
                pluginName = _path.default.basename(plugin, '.wasm');
            }
            return {
                eventName: EVENT_SWC_PLUGIN_PRESENT,
                payload: {
                    pluginName: pluginName,
                    pluginVersion: version
                }
            };
        });
    } catch (_) {
        return [];
    }
}

//# sourceMappingURL=swc-plugins.js.map