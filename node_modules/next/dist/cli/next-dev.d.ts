#!/usr/bin/env node
import '../server/lib/cpu-profile';
export type NextDevOptions = {
    disableSourceMaps: boolean;
    turbo?: boolean;
    turbopack?: boolean;
    port: number;
    hostname?: string;
    experimentalHttps?: boolean;
    experimentalHttpsKey?: string;
    experimentalHttpsCert?: string;
    experimentalHttpsCa?: string;
    experimentalUploadTrace?: string;
};
type PortSource = 'cli' | 'default' | 'env';
declare const nextDev: (options: NextDevOptions, portSource: PortSource, directory?: string) => Promise<void>;
export { nextDev };
