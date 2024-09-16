interface Options {
    /**
     * A transformer function that will be applied to the regexp generated
     * from the provided path and path-to-regexp.
     */
    regexModifier?: (regex: string) => string;
    /**
     * When true the function will remove all unnamed parameters
     * from the matched parameters.
     */
    removeUnnamedParams?: boolean;
    /**
     * When true the regexp won't allow an optional trailing delimiter
     * to match.
     */
    strict?: boolean;
    /**
     * When true the matcher will be case-sensitive, defaults to false
     */
    sensitive?: boolean;
}
export type PatchMatcher = (pathname?: string | null, params?: Record<string, any>) => Record<string, any> | false;
/**
 * Generates a path matcher function for a given path and options based on
 * path-to-regexp. By default the match will be case insensitive, non strict
 * and delimited by `/`.
 */
export declare function getPathMatch(path: string, options?: Options): PatchMatcher;
export {};
