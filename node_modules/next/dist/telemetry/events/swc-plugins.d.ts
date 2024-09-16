import type { NextConfig } from '../../server/config-shared';
type SwcPluginsEvent = {
    eventName: string;
    payload: {
        pluginName: string;
        pluginVersion?: string;
    };
};
export declare function eventSwcPlugins(dir: string, config: NextConfig): Promise<Array<SwcPluginsEvent>>;
export {};
