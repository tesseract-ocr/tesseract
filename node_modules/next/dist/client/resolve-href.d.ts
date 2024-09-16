import type { NextRouter, Url } from '../shared/lib/router/router';
/**
 * Resolves a given hyperlink with a certain router state (basePath not included).
 * Preserves absolute urls.
 */
export declare function resolveHref(router: NextRouter, href: Url, resolveAs: true): [string, string] | [string];
export declare function resolveHref(router: NextRouter, href: Url, resolveAs?: false): string;
