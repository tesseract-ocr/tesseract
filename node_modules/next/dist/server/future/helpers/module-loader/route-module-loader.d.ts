import type { RouteModule } from '../../route-modules/route-module';
import type { ModuleLoader } from './module-loader';
export interface AppLoaderModule<M extends RouteModule = RouteModule> {
    routeModule: M;
}
export declare class RouteModuleLoader {
    static load<M extends RouteModule>(id: string, loader?: ModuleLoader): Promise<M>;
}
