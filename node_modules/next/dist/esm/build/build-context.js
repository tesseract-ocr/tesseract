// A layer for storing data that is used by plugins to communicate with each
// other between different steps of the build process. This is only internal
// to Next.js and will not be a part of the final build output.
// These states don't need to be deeply merged.
let pluginState = {};
export function resumePluginState(resumedState) {
    Object.assign(pluginState, resumedState);
}
// This method gives you the plugin state with typed and mutable value fields
// behind a proxy so we can lazily initialize the values **after** resuming the
// plugin state.
export function getProxiedPluginState(initialState) {
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
export function getPluginState() {
    return pluginState;
}
// a global object to store context for the current build
// this is used to pass data between different steps of the build without having
// to pass it through function arguments.
// Not exhaustive, but should be extended to as needed whilst refactoring
export const NextBuildContext = {};

//# sourceMappingURL=build-context.js.map