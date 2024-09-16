/**
 * The default join function iterates over possible base paths until a suitable join is found.
 *
 * The first base path is used as fallback for the case where none of the base paths can locate the actual file.
 *
 * @type {function}
 */
export declare const defaultJoin: ((filename: string, options: {
    debug?: any | boolean;
    root: string;
}) => (uri: string, baseOrIteratorOrAbsent: any) => any) & ("" | {
    valueOf: () => string;
    toString: () => string;
});
