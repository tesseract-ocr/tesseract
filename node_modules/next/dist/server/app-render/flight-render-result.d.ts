import RenderResult from '../render-result';
/**
 * Flight Response is always set to RSC_CONTENT_TYPE_HEADER to ensure it does not get interpreted as HTML.
 */
export declare class FlightRenderResult extends RenderResult {
    constructor(response: string | ReadableStream<Uint8Array>);
}
