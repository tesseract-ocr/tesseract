import type { AppRouteHandlerFn, AppRouteHandlers } from '../module';
import { type HTTP_METHOD } from '../../../../web/http';
export declare function autoImplementMethods(handlers: AppRouteHandlers): Record<HTTP_METHOD, AppRouteHandlerFn>;
