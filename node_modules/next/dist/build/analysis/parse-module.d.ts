/**
 * Parses a module with SWC using an LRU cache where the parsed module will
 * be indexed by a sha of its content holding up to 500 entries.
 */
export declare const parseModule: (_: string, content: string) => Promise<any>;
