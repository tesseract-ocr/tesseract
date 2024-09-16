/// <reference types="node" />
export declare function getBlurImage(content: Buffer, extension: string, imageSize: {
    width: number;
    height: number;
}, { basePath, outputPath, isDev, tracing, }: {
    basePath: string;
    outputPath: string;
    isDev: boolean;
    tracing: (name?: string) => {
        traceFn(fn: Function): any;
        traceAsyncFn(fn: Function): any;
    };
}): Promise<{
    dataURL: string | undefined;
    width: number;
    height: number;
}>;
