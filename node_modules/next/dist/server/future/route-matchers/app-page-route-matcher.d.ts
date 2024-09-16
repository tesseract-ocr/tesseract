import { RouteMatcher } from './route-matcher';
import type { AppPageRouteDefinition } from '../route-definitions/app-page-route-definition';
export declare class AppPageRouteMatcher extends RouteMatcher<AppPageRouteDefinition> {
    get identity(): string;
}
