#!/usr/bin/env node
import '../server/lib/cpu-profile';
type NextStartOptions = {
    port: number;
    hostname?: string;
    keepAliveTimeout?: number;
};
declare const nextStart: (options: NextStartOptions, directory?: string) => Promise<void>;
export { nextStart };
