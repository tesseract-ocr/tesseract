import type { ManifestHeaderRoute, ManifestRedirectRoute, ManifestRewriteRoute } from '../build';
import { type Header, type Redirect, type Rewrite } from './load-custom-routes';
export declare function buildCustomRoute(type: 'header', route: Header): ManifestHeaderRoute;
export declare function buildCustomRoute(type: 'rewrite', route: Rewrite): ManifestRewriteRoute;
export declare function buildCustomRoute(type: 'redirect', route: Redirect, restrictedRedirectPaths: string[]): ManifestRedirectRoute;
