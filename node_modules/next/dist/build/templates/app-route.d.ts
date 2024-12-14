import { AppRouteRouteModule } from '../../server/route-modules/app-route/module.compiled';
declare const routeModule: AppRouteRouteModule;
declare const workAsyncStorage: import("../../server/app-render/work-async-storage.external").WorkAsyncStorage, workUnitAsyncStorage: import("../../server/app-render/work-unit-async-storage.external").WorkUnitAsyncStorage, serverHooks: typeof import("../../client/components/hooks-server-context");
declare function patchFetch(): void;
export { routeModule, workAsyncStorage, workUnitAsyncStorage, serverHooks, patchFetch, };
