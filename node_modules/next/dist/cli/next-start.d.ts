#!/usr/bin/env node
import '../server/lib/cpu-profile';
export type NextStartOptions = {
    port: number;
    hostname?: string;
    keepAliveTimeout?: number;
};
/**
 * Start the Next.js server
 *
 * @param options The options for the start command
 * @param directory The directory to start the server in
 */
declare const nextStart: (options: NextStartOptions, directory?: string) => Promise<void>;
export { nextStart };
