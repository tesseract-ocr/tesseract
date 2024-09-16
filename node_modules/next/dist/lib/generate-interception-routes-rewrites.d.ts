import type { Rewrite } from './load-custom-routes';
export declare function generateInterceptionRoutesRewrites(appPaths: string[], basePath?: string): Rewrite[];
export declare function isInterceptionRouteRewrite(route: Rewrite): boolean;
