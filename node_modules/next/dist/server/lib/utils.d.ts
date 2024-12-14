export declare function printAndExit(message: string, code?: number): never;
/**
 * Tokenizes the arguments string into an array of strings, supporting quoted
 * values and escaped characters.
 * Converted from: https://github.com/nodejs/node/blob/c29d53c5cfc63c5a876084e788d70c9e87bed880/src/node_options.cc#L1401
 *
 * @param input The arguments string to be tokenized.
 * @returns An array of strings with the tokenized arguments.
 */
export declare const tokenizeArgs: (input: string) => string[];
/**
 * The debug address is in the form of `[host:]port`. The host is optional.
 */
type DebugAddress = {
    host: string | undefined;
    port: number;
};
/**
 * Formats the debug address into a string.
 */
export declare const formatDebugAddress: ({ host, port }: DebugAddress) => string;
/**
 * Get's the debug address from the `NODE_OPTIONS` environment variable. If the
 * address is not found, it returns the default host (`undefined`) and port
 * (`9229`).
 *
 * @returns An object with the host and port of the debug address.
 */
export declare const getParsedDebugAddress: () => DebugAddress;
/**
 * Get the debug address from the `NODE_OPTIONS` environment variable and format
 * it into a string.
 *
 * @returns A string with the formatted debug address.
 */
export declare const getFormattedDebugAddress: () => string;
/**
 * Stringify the arguments to be used in a command line. It will ignore any
 * argument that has a value of `undefined`.
 *
 * @param args The arguments to be stringified.
 * @returns A string with the arguments.
 */
export declare function formatNodeOptions(args: Record<string, string | boolean | undefined>): string;
/**
 * Get the node options from the `NODE_OPTIONS` environment variable and parse
 * them into an object without the inspect options.
 *
 * @returns An object with the parsed node options.
 */
export declare function getParsedNodeOptionsWithoutInspect(): {
    [longOption: string]: string | boolean | undefined;
};
/**
 * Get the node options from the `NODE_OPTIONS` environment variable and format
 * them into a string without the inspect options.
 *
 * @returns A string with the formatted node options.
 */
export declare function getFormattedNodeOptionsWithoutInspect(): string;
/**
 * Check if the value is a valid positive integer and parse it. If it's not, it will throw an error.
 *
 * @param value The value to be parsed.
 */
export declare function parseValidPositiveInteger(value: string): number;
export declare const RESTART_EXIT_CODE = 77;
export type NodeInspectType = 'inspect' | 'inspect-brk' | undefined;
/**
 * Get the debug type from the `NODE_OPTIONS` environment variable.
 */
export declare function getNodeDebugType(): NodeInspectType;
/**
 * Get the `max-old-space-size` value from the `NODE_OPTIONS` environment
 * variable.
 *
 * @returns The value of the `max-old-space-size` option as a number.
 */
export declare function getMaxOldSpaceSize(): number | undefined;
export {};
