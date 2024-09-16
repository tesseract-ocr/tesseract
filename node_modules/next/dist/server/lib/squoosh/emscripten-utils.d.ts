export declare function pathify(path: string): string;
export declare function instantiateEmscriptenWasm<T extends EmscriptenWasm.Module>(factory: EmscriptenWasm.ModuleFactory<T>, path: string, workerJS?: string): Promise<T>;
