import type { ModuleLoader } from './module-loader';
/**
 * Loads a module using `await require(id)`.
 */
export declare class NodeModuleLoader implements ModuleLoader {
    load<M>(id: string): Promise<M>;
}
