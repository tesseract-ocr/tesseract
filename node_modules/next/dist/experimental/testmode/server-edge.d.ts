export declare function interceptTestApis(): () => void;
export declare function wrapRequestHandler<T>(handler: (req: Request, fn: () => T) => T): (req: Request, fn: () => T) => T;
