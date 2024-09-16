import type { AppRouteRouteModule } from './app-route/module';
import type { AppPageRouteModule } from './app-page/module';
import type { PagesRouteModule } from './pages/module';
import type { PagesAPIRouteModule } from './pages-api/module';
import type { RouteModule } from './route-module';
export declare function isAppRouteRouteModule(routeModule: RouteModule): routeModule is AppRouteRouteModule;
export declare function isAppPageRouteModule(routeModule: RouteModule): routeModule is AppPageRouteModule;
export declare function isPagesRouteModule(routeModule: RouteModule): routeModule is PagesRouteModule;
export declare function isPagesAPIRouteModule(routeModule: RouteModule): routeModule is PagesAPIRouteModule;
