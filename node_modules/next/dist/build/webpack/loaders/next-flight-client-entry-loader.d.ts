import type { webpack } from 'next/dist/compiled/webpack/webpack';
/**
 * { [client import path]: [exported names] }
 */
export type ClientComponentImports = Record<string, Set<string>>;
export type CssImports = Record<string, string[]>;
export type NextFlightClientEntryLoaderOptions = {
    modules: string[] | string;
    /** This is transmitted as a string to `getOptions` */
    server: boolean | 'true' | 'false';
};
export type FlightClientEntryModuleItem = {
    request: string;
    ids: string[];
};
export default function transformSource(this: webpack.LoaderContext<NextFlightClientEntryLoaderOptions>): string;
