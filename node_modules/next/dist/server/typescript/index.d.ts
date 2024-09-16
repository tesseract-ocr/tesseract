/**
 * This is a TypeScript language service plugin for Next.js app directory,
 * it provides the following features:
 *
 * - Warns about disallowed React APIs in server components.
 * - Warns about disallowed layout and page exports.
 * - Autocompletion for entry configurations.
 * - Hover hint and docs for entry configurations.
 */
import type tsModule from 'typescript/lib/tsserverlibrary';
export declare const createTSPlugin: tsModule.server.PluginModuleFactory;
