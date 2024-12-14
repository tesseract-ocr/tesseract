"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "loadDefaultErrorComponents", {
    enumerable: true,
    get: function() {
        return loadDefaultErrorComponents;
    }
});
const _constants = require("../shared/lib/constants");
const _path = require("path");
const _interopdefault = require("../lib/interop-default");
const _tracer = require("./lib/trace/tracer");
const _constants1 = require("./lib/trace/constants");
const _loadcomponents = require("./load-components");
async function loadDefaultErrorComponentsImpl(distDir) {
    const Document = (0, _interopdefault.interopDefault)(require('next/dist/pages/_document'));
    const AppMod = require('next/dist/pages/_app');
    const App = (0, _interopdefault.interopDefault)(AppMod);
    // Load the compiled route module for this builtin error.
    // TODO: (wyattjoh) replace this with just exporting the route module when the transition is complete
    const ComponentMod = require('./route-modules/pages/builtin/_error');
    const Component = ComponentMod.routeModule.userland.default;
    return {
        App,
        Document,
        Component,
        pageConfig: {},
        buildManifest: await (0, _loadcomponents.loadManifestWithRetries)((0, _path.join)(distDir, `fallback-${_constants.BUILD_MANIFEST}`)),
        reactLoadableManifest: {},
        ComponentMod,
        page: '/_error',
        routeModule: ComponentMod.routeModule
    };
}
const loadDefaultErrorComponents = (0, _tracer.getTracer)().wrap(_constants1.LoadComponentsSpan.loadDefaultErrorComponents, loadDefaultErrorComponentsImpl);

//# sourceMappingURL=load-default-error-components.js.map