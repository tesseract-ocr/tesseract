"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "eventNextPlugins", {
    enumerable: true,
    get: function() {
        return eventNextPlugins;
    }
});
const _findup = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/find-up"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const EVENT_PLUGIN_PRESENT = "NEXT_PACKAGE_DETECTED";
async function eventNextPlugins(dir) {
    try {
        const packageJsonPath = await (0, _findup.default)("package.json", {
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
        return Object.keys(deps).reduce((events, plugin)=>{
            const version = deps[plugin];
            // Don't add deps without a version set
            if (!version) {
                return events;
            }
            events.push({
                eventName: EVENT_PLUGIN_PRESENT,
                payload: {
                    packageName: plugin,
                    packageVersion: version
                }
            });
            return events;
        }, []);
    } catch (_) {
        return [];
    }
}

//# sourceMappingURL=plugins.js.map