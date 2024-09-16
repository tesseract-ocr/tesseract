export declare const INTERCEPTION_ROUTE_MARKERS: readonly ["(..)(..)", "(.)", "(..)", "(...)"];
export declare function isInterceptionRouteAppPath(path: string): boolean;
export declare function extractInterceptionRouteInformation(path: string): {
    interceptingRoute: string;
    interceptedRoute: string;
};
