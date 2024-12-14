import type { webpack } from 'next/dist/compiled/webpack/webpack';
import type { ResolvedBaseUrl } from '../../load-jsconfig';
export interface Pattern {
    prefix: string;
    suffix: string;
}
export declare function hasZeroOrOneAsteriskCharacter(str: string): boolean;
/**
 * Determines whether a path starts with a relative path component (i.e. `.` or `..`).
 */
export declare function pathIsRelative(testPath: string): boolean;
export declare function tryParsePattern(pattern: string): Pattern | undefined;
/** Return the object corresponding to the best pattern to match `candidate`. */
export declare function findBestPatternMatch<T>(values: readonly T[], getPattern: (value: T) => Pattern, candidate: string): T | undefined;
/**
 * patternStrings contains both pattern strings (containing "*") and regular strings.
 * Return an exact match if possible, or a pattern match, or undefined.
 * (These are verified by verifyCompilerOptions to have 0 or 1 "*" characters.)
 */
export declare function matchPatternOrExact(patternStrings: readonly string[], candidate: string): string | Pattern | undefined;
/**
 * Tests whether a value is string
 */
export declare function isString(text: unknown): text is string;
/**
 * Given that candidate matches pattern, returns the text matching the '*'.
 * E.g.: matchedText(tryParsePattern("foo*baz"), "foobarbaz") === "bar"
 */
export declare function matchedText(pattern: Pattern, candidate: string): string;
export declare function patternText({ prefix, suffix }: Pattern): string;
type Paths = {
    [match: string]: string[];
};
/**
 * Handles tsconfig.json or jsconfig.js "paths" option for webpack
 * Largely based on how the TypeScript compiler handles it:
 * https://github.com/microsoft/TypeScript/blob/1a9c8197fffe3dace5f8dca6633d450a88cba66d/src/compiler/moduleNameResolver.ts#L1362
 */
type NonFunction<T> = T extends Function ? never : T;
type ResolvePluginPlugin = NonFunction<webpack.ResolvePluginInstance>;
export declare class JsConfigPathsPlugin implements ResolvePluginPlugin {
    paths: Paths;
    resolvedBaseUrl: ResolvedBaseUrl;
    jsConfigPlugin: true;
    constructor(paths: Paths, resolvedBaseUrl: ResolvedBaseUrl);
    apply(resolver: webpack.Resolver): void;
}
export {};
