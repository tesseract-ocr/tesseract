import { PassThrough } from 'stream';
export function requestToBodyStream(context, KUint8Array, stream) {
    return new context.ReadableStream({
        start: async (controller)=>{
            for await (const chunk of stream){
                controller.enqueue(new KUint8Array(chunk));
            }
            controller.close();
        }
    });
}
function replaceRequestBody(base, stream) {
    for(const key in stream){
        let v = stream[key];
        if (typeof v === 'function') {
            v = v.bind(base);
        }
        base[key] = v;
    }
    return base;
}
export function getCloneableBody(readable) {
    let buffered = null;
    const endPromise = new Promise((resolve, reject)=>{
        readable.on('end', resolve);
        readable.on('error', reject);
    }).catch((error)=>{
        return {
            error
        };
    });
    return {
        /**
     * Replaces the original request body if necessary.
     * This is done because once we read the body from the original request,
     * we can't read it again.
     */ async finalize () {
            if (buffered) {
                const res = await endPromise;
                if (res && typeof res === 'object' && res.error) {
                    throw res.error;
                }
                replaceRequestBody(readable, buffered);
                buffered = readable;
            }
        },
        /**
     * Clones the body stream
     * to pass into a middleware
     */ cloneBodyStream () {
            const input = buffered ?? readable;
            const p1 = new PassThrough();
            const p2 = new PassThrough();
            input.on('data', (chunk)=>{
                p1.push(chunk);
                p2.push(chunk);
            });
            input.on('end', ()=>{
                p1.push(null);
                p2.push(null);
            });
            buffered = p2;
            return p1;
        }
    };
}

//# sourceMappingURL=body-streams.js.map