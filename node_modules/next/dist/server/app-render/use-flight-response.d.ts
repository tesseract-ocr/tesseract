import type { ClientReferenceManifest } from '../../build/webpack/plugins/flight-manifest-plugin';
import type { BinaryStreamOf } from './app-render';
import type { DeepReadonly } from '../../shared/lib/deep-readonly';
/**
 * Render Flight stream.
 * This is only used for renderToHTML, the Flight response does not need additional wrappers.
 */
export declare function useFlightStream<T>(flightStream: BinaryStreamOf<T>, clientReferenceManifest: DeepReadonly<ClientReferenceManifest>, nonce?: string): Promise<T>;
/**
 * There are times when an SSR render may be finished but the RSC render
 * is ongoing and we need to wait for it to complete to make some determination
 * about how to handle the render. This function will drain the RSC reader and
 * resolve when completed. This will generally require teeing the RSC stream and it
 * should be noted that it will cause all the RSC chunks to queue in the underlying
 * ReadableStream however given Flight currently is a push stream that doesn't respond
 * to backpressure this shouldn't change how much memory is maximally consumed
 */
export declare function flightRenderComplete(flightStream: ReadableStream<Uint8Array>): Promise<void>;
/**
 * Creates a ReadableStream provides inline script tag chunks for writing hydration
 * data to the client outside the React render itself.
 *
 * @param flightStream The RSC render stream
 * @param nonce optionally a nonce used during this particular render
 * @param formState optionally the formState used with this particular render
 * @returns a ReadableStream without the complete property. This signifies a lazy ReadableStream
 */
export declare function createInlinedDataReadableStream(flightStream: ReadableStream<Uint8Array>, nonce: string | undefined, formState: unknown | null): ReadableStream<Uint8Array>;
