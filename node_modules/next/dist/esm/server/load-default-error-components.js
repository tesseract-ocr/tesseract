import { BUILD_MANIFEST } from "../shared/lib/constants";
import { join } from "path";
import { interopDefault } from "../lib/interop-default";
import { getTracer } from "./lib/trace/tracer";
import { LoadComponentsSpan } from "./lib/trace/constants";
import { loadManifestWithRetries } from "./load-components";
async function loadDefaultErrorComponentsImpl(distDir) {
    const Document = interopDefault(require("next/dist/pages/_document"));
    const AppMod = require("next/dist/pages/_app");
    const App = interopDefault(AppMod);
    // Load the compiled route module for this builtin error.
    // TODO: (wyattjoh) replace this with just exporting the route module when the transition is complete
    const ComponentMod = require("./future/route-modules/pages/builtin/_error");
    const Component = ComponentMod.routeModule.userland.default;
    return {
        App,
        Document,
        Component,
        pageConfig: {},
        buildManifest: await loadManifestWithRetries(join(distDir, `fallback-${BUILD_MANIFEST}`)),
        reactLoadableManifest: {},
        ComponentMod,
        page: "/_error",
        routeModule: ComponentMod.routeModule
    };
}
export const loadDefaultErrorComponents = getTracer().wrap(LoadComponentsSpan.loadDefaultErrorComponents, loadDefaultErrorComponentsImpl);

//# sourceMappingURL=load-default-error-components.js.map