/// <reference types="react" />
export type ReactReadableStream = ReadableStream<Uint8Array> & {
    allReady?: Promise<void> | undefined;
};
export declare function chainStreams<T>(...streams: ReadableStream<T>[]): ReadableStream<T>;
export declare function streamFromString(str: string): ReadableStream<Uint8Array>;
export declare function streamToString(stream: ReadableStream<Uint8Array>): Promise<string>;
export declare function createBufferedTransformStream(): TransformStream<Uint8Array, Uint8Array>;
export declare function renderToInitialFizzStream({ ReactDOMServer, element, streamOptions, }: {
    ReactDOMServer: typeof import('react-dom/server.edge');
    element: React.ReactElement;
    streamOptions?: any;
}): Promise<ReactReadableStream>;
export declare function createRootLayoutValidatorStream(): TransformStream<Uint8Array, Uint8Array>;
export type ContinueStreamOptions = {
    inlinedDataStream: ReadableStream<Uint8Array> | undefined;
    isStaticGeneration: boolean;
    getServerInsertedHTML: (() => Promise<string>) | undefined;
    serverInsertedHTMLToHead: boolean;
    validateRootLayout?: boolean;
    /**
     * Suffix to inject after the buffered data, but before the close tags.
     */
    suffix?: string | undefined;
};
export declare function continueFizzStream(renderStream: ReactReadableStream, { suffix, inlinedDataStream, isStaticGeneration, getServerInsertedHTML, serverInsertedHTMLToHead, validateRootLayout, }: ContinueStreamOptions): Promise<ReadableStream<Uint8Array>>;
type ContinueDynamicPrerenderOptions = {
    getServerInsertedHTML: () => Promise<string>;
};
export declare function continueDynamicPrerender(prerenderStream: ReadableStream<Uint8Array>, { getServerInsertedHTML }: ContinueDynamicPrerenderOptions): Promise<ReadableStream<Uint8Array>>;
type ContinueStaticPrerenderOptions = {
    inlinedDataStream: ReadableStream<Uint8Array>;
    getServerInsertedHTML: () => Promise<string>;
};
export declare function continueStaticPrerender(prerenderStream: ReadableStream<Uint8Array>, { inlinedDataStream, getServerInsertedHTML }: ContinueStaticPrerenderOptions): Promise<ReadableStream<Uint8Array>>;
type ContinueResumeOptions = {
    inlinedDataStream: ReadableStream<Uint8Array>;
    getServerInsertedHTML: () => Promise<string>;
};
export declare function continueDynamicHTMLResume(renderStream: ReadableStream<Uint8Array>, { inlinedDataStream, getServerInsertedHTML }: ContinueResumeOptions): Promise<ReadableStream<Uint8Array>>;
type ContinueDynamicDataResumeOptions = {
    inlinedDataStream: ReadableStream<Uint8Array>;
};
export declare function continueDynamicDataResume(renderStream: ReadableStream<Uint8Array>, { inlinedDataStream }: ContinueDynamicDataResumeOptions): Promise<ReadableStream<Uint8Array>>;
export {};
