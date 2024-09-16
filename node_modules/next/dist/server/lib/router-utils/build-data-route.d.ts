export declare function buildDataRoute(page: string, buildId: string): {
    page: string;
    routeKeys: {
        [named: string]: string;
    } | undefined;
    dataRouteRegex: string;
    namedDataRouteRegex: string | undefined;
};
