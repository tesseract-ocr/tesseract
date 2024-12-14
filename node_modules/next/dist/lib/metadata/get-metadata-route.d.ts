/**
 * Fill the dynamic segment in the metadata route
 *
 * Example:
 * fillMetadataSegment('/a/[slug]', { params: { slug: 'b' } }, 'open-graph') -> '/a/b/open-graph'
 *
 */
export declare function fillMetadataSegment(segment: string, params: any, lastSegment: string): string;
/**
 * Map metadata page key to the corresponding route
 *
 * static file page key:    /app/robots.txt -> /robots.xml -> /robots.txt/route
 * dynamic route page key:  /app/robots.tsx -> /robots -> /robots.txt/route
 *
 * @param page
 * @returns
 */
export declare function normalizeMetadataRoute(page: string): string;
export declare function normalizeMetadataPageToRoute(page: string, isDynamic: boolean): string;
