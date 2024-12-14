var _self___RSC_MANIFEST;
import { createServerModuleMap } from '../../server/app-render/action-utils';
import { setReferenceManifestsSingleton } from '../../server/app-render/encryption-utils';
import { EdgeRouteModuleWrapper } from '../../server/web/edge-route-module-wrapper';
// Import the userland code.
import * as module from 'VAR_USERLAND';
// INJECT:nextConfig
const maybeJSONParse = (str)=>str ? JSON.parse(str) : undefined;
const rscManifest = (_self___RSC_MANIFEST = self.__RSC_MANIFEST) == null ? void 0 : _self___RSC_MANIFEST['VAR_PAGE'];
const rscServerManifest = maybeJSONParse(self.__RSC_SERVER_MANIFEST);
if (rscManifest && rscServerManifest) {
    setReferenceManifestsSingleton({
        page: 'VAR_PAGE',
        clientReferenceManifest: rscManifest,
        serverActionsManifest: rscServerManifest,
        serverModuleMap: createServerModuleMap({
            serverActionsManifest: rscServerManifest
        })
    });
}
export const ComponentMod = module;
export default EdgeRouteModuleWrapper.wrap(module.routeModule, {
    nextConfig
});

//# sourceMappingURL=edge-app-route.js.map