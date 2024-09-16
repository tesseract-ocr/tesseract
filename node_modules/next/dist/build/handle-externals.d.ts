import type { WebpackLayerName } from '../lib/constants';
import type { NextConfigComplete } from '../server/config-shared';
import type { ResolveOptions } from 'webpack';
export declare function isResourceInPackages(resource: string, packageNames?: string[], packageDirMapping?: Map<string, string>): boolean;
export declare function resolveExternal(dir: string, esmExternalsConfig: NextConfigComplete['experimental']['esmExternals'], context: string, request: string, isEsmRequested: boolean, _optOutBundlingPackages: string[], getResolve: (options: ResolveOptions) => (resolveContext: string, resolveRequest: string) => Promise<[string | null, boolean]>, isLocalCallback?: (res: string) => any, baseResolveCheck?: boolean, esmResolveOptions?: any, nodeResolveOptions?: any, baseEsmResolveOptions?: any, baseResolveOptions?: any): Promise<{
    localRes: any;
    res?: undefined;
    isEsm?: undefined;
} | {
    res: string | null;
    isEsm: boolean;
    localRes?: undefined;
}>;
export declare function makeExternalHandler({ config, optOutBundlingPackages, optOutBundlingPackageRegex, dir, }: {
    config: NextConfigComplete;
    optOutBundlingPackages: string[];
    optOutBundlingPackageRegex: RegExp;
    dir: string;
}): (context: string, request: string, dependencyType: string, layer: WebpackLayerName | null, getResolve: (options: any) => (resolveContext: string, resolveRequest: string) => Promise<[string | null, boolean]>) => Promise<any>;
