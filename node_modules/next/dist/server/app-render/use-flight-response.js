"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    createInlinedDataReadableStream: null,
    flightRenderComplete: null,
    useFlightStream: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    createInlinedDataReadableStream: function() {
        return createInlinedDataReadableStream;
    },
    flightRenderComplete: function() {
        return flightRenderComplete;
    },
    useFlightStream: function() {
        return useFlightStream;
    }
});
const _htmlescape = require("../htmlescape");
const isEdgeRuntime = process.env.NEXT_RUNTIME === "edge";
const INLINE_FLIGHT_PAYLOAD_BOOTSTRAP = 0;
const INLINE_FLIGHT_PAYLOAD_DATA = 1;
const INLINE_FLIGHT_PAYLOAD_FORM_STATE = 2;
const flightResponses = new WeakMap();
const encoder = new TextEncoder();
function useFlightStream(flightStream, clientReferenceManifest, nonce) {
    const response = flightResponses.get(flightStream);
    if (response) {
        return response;
    }
    // react-server-dom-webpack/client.edge must not be hoisted for require cache clearing to work correctly
    let createFromReadableStream;
    // @TODO: investigate why the aliasing for turbopack doesn't pick this up, requiring this runtime check
    if (process.env.TURBOPACK) {
        createFromReadableStream = // eslint-disable-next-line import/no-extraneous-dependencies
        require("react-server-dom-turbopack/client.edge").createFromReadableStream;
    } else {
        createFromReadableStream = // eslint-disable-next-line import/no-extraneous-dependencies
        require("react-server-dom-webpack/client.edge").createFromReadableStream;
    }
    const newResponse = createFromReadableStream(flightStream, {
        ssrManifest: {
            moduleLoading: clientReferenceManifest.moduleLoading,
            moduleMap: isEdgeRuntime ? clientReferenceManifest.edgeSSRModuleMapping : clientReferenceManifest.ssrModuleMapping
        },
        nonce
    });
    flightResponses.set(flightStream, newResponse);
    return newResponse;
}
async function flightRenderComplete(flightStream) {
    const flightReader = flightStream.getReader();
    while(true){
        const { done } = await flightReader.read();
        if (done) {
            return;
        }
    }
}
function createInlinedDataReadableStream(flightStream, nonce, formState) {
    const startScriptTag = nonce ? `<script nonce=${JSON.stringify(nonce)}>` : "<script>";
    const decoder = new TextDecoder("utf-8", {
        fatal: true
    });
    const decoderOptions = {
        stream: true
    };
    const flightReader = flightStream.getReader();
    const readable = new ReadableStream({
        type: "bytes",
        start (controller) {
            try {
                writeInitialInstructions(controller, startScriptTag, formState);
            } catch (error) {
                // during encoding or enqueueing forward the error downstream
                controller.error(error);
            }
        },
        async pull (controller) {
            try {
                const { done, value } = await flightReader.read();
                if (done) {
                    const tail = decoder.decode(value, {
                        stream: false
                    });
                    if (tail.length) {
                        writeFlightDataInstruction(controller, startScriptTag, tail);
                    }
                    controller.close();
                } else {
                    const chunkAsString = decoder.decode(value, decoderOptions);
                    writeFlightDataInstruction(controller, startScriptTag, chunkAsString);
                }
            } catch (error) {
                // There was a problem in the upstream reader or during decoding or enqueuing
                // forward the error downstream
                controller.error(error);
            }
        }
    });
    return readable;
}
function writeInitialInstructions(controller, scriptStart, formState) {
    controller.enqueue(encoder.encode(`${scriptStart}(self.__next_f=self.__next_f||[]).push(${(0, _htmlescape.htmlEscapeJsonString)(JSON.stringify([
        INLINE_FLIGHT_PAYLOAD_BOOTSTRAP
    ]))});self.__next_f.push(${(0, _htmlescape.htmlEscapeJsonString)(JSON.stringify([
        INLINE_FLIGHT_PAYLOAD_FORM_STATE,
        formState
    ]))})</script>`));
}
function writeFlightDataInstruction(controller, scriptStart, chunkAsString) {
    controller.enqueue(encoder.encode(`${scriptStart}self.__next_f.push(${(0, _htmlescape.htmlEscapeJsonString)(JSON.stringify([
        INLINE_FLIGHT_PAYLOAD_DATA,
        chunkAsString
    ]))})</script>`));
}

//# sourceMappingURL=use-flight-response.js.map