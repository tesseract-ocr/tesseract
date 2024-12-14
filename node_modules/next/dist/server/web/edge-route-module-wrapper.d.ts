import type { AppRouteRouteModule } from '../route-modules/app-route/module';
import './globals';
import { type AdapterOptions } from './adapter';
import type { NextConfigComplete } from '../config-shared';
export interface WrapOptions {
    nextConfig: NextConfigComplete;
}
/**
 * EdgeRouteModuleWrapper is a wrapper around a route module.
 *
 * Note that this class should only be used in the edge runtime.
 */
export declare class EdgeRouteModuleWrapper {
    private readonly routeModule;
    private readonly nextConfig;
    private readonly matcher;
    /**
     * The constructor is wrapped with private to ensure that it can only be
     * constructed by the static wrap method.
     *
     * @param routeModule the route module to wrap
     */
    private constructor();
    /**
     * This will wrap a module with the EdgeModuleWrapper and return a function
     * that can be used as a handler for the edge runtime.
     *
     * @param module the module to wrap
     * @param options any options that should be passed to the adapter and
     *                override the ones passed from the runtime
     * @returns a function that can be used as a handler for the edge runtime
     */
    static wrap(routeModule: AppRouteRouteModule, options: WrapOptions): (opts: AdapterOptions) => Promise<import("./types").FetchEventResult>;
    private handler;
}
