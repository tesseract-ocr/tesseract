"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    NextBuildContext: null,
    getPluginState: null,
    getProxiedPluginState: null,
    resumePluginState: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    NextBuildContext: function() {
        return NextBuildContext;
    },
    getPluginState: function() {
        return getPluginState;
    },
    getProxiedPluginState: function() {
        return getProxiedPluginState;
    },
    resumePluginState: function() {
        return resumePluginState;
    }
});
// A layer for storing data that is used by plugins to communicate with each
// other between different steps of the build process. This is only internal
// to Next.js and will not be a part of the final build output.
// These states don't need to be deeply merged.
let pluginState = {};
function resumePluginState(resumedState) {
    Object.assign(pluginState, resumedState);
}
function getProxiedPluginState(initialState) {
    return new Proxy(pluginState, {
        get (target, key) {
            if (typeof target[key] === 'undefined') {
                return target[key] = initialState[key];
            }
            return target[key];
        },
        set (target, key, value) {
            target[key] = value;
            return true;
        }
    });
}
function getPluginState() {
    return pluginState;
}
const NextBuildContext = {};

//# sourceMappingURL=build-context.js.map