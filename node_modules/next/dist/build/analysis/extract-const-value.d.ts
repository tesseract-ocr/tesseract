import type { Module } from '@swc/core';
export declare class NoSuchDeclarationError extends Error {
}
export declare class UnsupportedValueError extends Error {
    /** @example `config.runtime[0].value` */
    path?: string;
    constructor(message: string, paths?: string[]);
}
/**
 * Extracts the value of an exported const variable named `exportedName`
 * (e.g. "export const config = { runtime: 'edge' }") from swc's AST.
 * The value must be one of (or throws UnsupportedValueError):
 *   - string
 *   - boolean
 *   - number
 *   - null
 *   - undefined
 *   - array containing values listed in this list
 *   - object containing values listed in this list
 *
 * Throws NoSuchDeclarationError if the declaration is not found.
 */
export declare function extractExportedConstValue(module: Module, exportedName: string): any;
