/**
 * A webpack loader that resolves absolute url() paths relative to their original source file.
 * Requires source-maps to do any meaningful work.
 */
export default function resolveUrlLoader(this: any, 
/** Css content */
content: string, 
/** The source-map */
sourceMap: any): Promise<void>;
