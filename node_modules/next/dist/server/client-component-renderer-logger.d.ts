export declare function wrapClientComponentLoader(ComponentMod: any): any;
export declare function getClientComponentLoaderMetrics(options?: {
    reset?: boolean;
}): {
    clientComponentLoadStart: number;
    clientComponentLoadTimes: number;
    clientComponentLoadCount: number;
} | undefined;
