import type { ParsedUrlQuery } from 'querystring';
import type { Rewrite } from '../../../../lib/load-custom-routes';
import { type ParsedRelativeUrl } from './parse-relative-url';
export default function resolveRewrites(asPath: string, pages: string[], rewrites: {
    beforeFiles: Rewrite[];
    afterFiles: Rewrite[];
    fallback: Rewrite[];
}, query: ParsedUrlQuery, resolveHref: (path: string) => string, locales?: string[]): {
    matchedPage: boolean;
    parsedAs: ParsedRelativeUrl;
    asPath: string;
    resolvedHref?: string;
    externalDest?: boolean;
};
