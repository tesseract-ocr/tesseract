import type { MetadataRoute } from '../../../../lib/metadata/types/metadata-interface';
export declare function resolveRobots(data: MetadataRoute.Robots): string;
export declare function resolveSitemap(data: MetadataRoute.Sitemap): string;
export declare function resolveManifest(data: MetadataRoute.Manifest): string;
export declare function resolveRouteData(data: MetadataRoute.Robots | MetadataRoute.Sitemap | MetadataRoute.Manifest, fileType: 'robots' | 'sitemap' | 'manifest'): string;
