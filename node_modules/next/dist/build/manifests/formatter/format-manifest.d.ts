/**
 * Formats the manifest depending on the environment variable
 * `NODE_ENV`. If it's set to `development`, it will return a pretty printed
 * JSON string, otherwise it will return a minified JSON string.
 */
export declare function formatManifest<T extends object>(manifest: T): string;
