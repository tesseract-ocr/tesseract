type NextPluginsEvent = {
    eventName: string;
    payload: {
        packageName: string;
        packageVersion: string;
    };
};
export declare function eventNextPlugins(dir: string): Promise<Array<NextPluginsEvent>>;
export {};
