import type { IncomingMessage } from 'http';
import type { Readable } from 'stream';
export declare function requestToBodyStream(context: {
    ReadableStream: typeof ReadableStream;
}, KUint8Array: typeof Uint8Array, stream: Readable): ReadableStream<any>;
export interface CloneableBody {
    finalize(): Promise<void>;
    cloneBodyStream(): Readable;
}
export declare function getCloneableBody<T extends IncomingMessage>(readable: T): CloneableBody;
