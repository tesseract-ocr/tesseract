export declare const allowedStatusCodes: Set<number>;
export declare function getRedirectStatus(route: {
    statusCode?: number;
    permanent?: boolean;
}): number;
export declare function modifyRouteRegex(regex: string, restrictedPaths?: string[]): string;
