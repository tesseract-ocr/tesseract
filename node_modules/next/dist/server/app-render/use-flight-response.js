"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    createInlinedDataReadableStream: null,
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
    useFlightStream: function() {
        return useFlightStream;
    }
});
const _htmlescape = require("../htmlescape");
const isEdgeRuntime = process.env.NEXT_RUNTIME === 'edge';
const INLINE_FLIGHT_PAYLOAD_BOOTSTRAP = 0;
const INLINE_FLIGHT_PAYLOAD_DATA = 1;
const INLINE_FLIGHT_PAYLOAD_FORM_STATE = 2;
const INLINE_FLIGHT_PAYLOAD_BINARY = 3;
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
        require('react-server-dom-turbopack/client.edge').createFromReadableStream;
    } else {
        createFromReadableStream = // eslint-disable-next-line import/no-extraneous-dependencies
        require('react-server-dom-webpack/client.edge').createFromReadableStream;
    }
    const newResponse = createFromReadableStream(flightStream, {
        serverConsumerManifest: {
            moduleLoading: clientReferenceManifest.moduleLoading,
            moduleMap: isEdgeRuntime ? clientReferenceManifest.edgeSSRModuleMapping : clientReferenceManifest.ssrModuleMapping,
            serverModuleMap: null
        },
        nonce
    });
    flightResponses.set(flightStream, newResponse);
    return newResponse;
}
function createInlinedDataReadableStream(flightStream, nonce, formState) {
    const startScriptTag = nonce ? `<script nonce=${JSON.stringify(nonce)}>` : '<script>';
    const flightReader = flightStream.getReader();
    const decoder = new TextDecoder('utf-8', {
        fatal: true
    });
    const readable = new ReadableStream({
        type: 'bytes',
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
                if (value) {
                    try {
                        const decodedString = decoder.decode(value, {
                            stream: !done
                        });
                        // The chunk cannot be decoded as valid UTF-8 string as it might
                        // have arbitrary binary data.
                        writeFlightDataInstruction(controller, startScriptTag, decodedString);
                    } catch  {
                        // The chunk cannot be decoded as valid UTF-8 string.
                        writeFlightDataInstruction(controller, startScriptTag, value);
                    }
                }
                if (done) {
                    controller.close();
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
    if (formState != null) {
        controller.enqueue(encoder.encode(`${scriptStart}(self.__next_f=self.__next_f||[]).push(${(0, _htmlescape.htmlEscapeJsonString)(JSON.stringify([
            INLINE_FLIGHT_PAYLOAD_BOOTSTRAP
        ]))});self.__next_f.push(${(0, _htmlescape.htmlEscapeJsonString)(JSON.stringify([
            INLINE_FLIGHT_PAYLOAD_FORM_STATE,
            formState
        ]))})</script>`));
    } else {
        controller.enqueue(encoder.encode(`${scriptStart}(self.__next_f=self.__next_f||[]).push(${(0, _htmlescape.htmlEscapeJsonString)(JSON.stringify([
            INLINE_FLIGHT_PAYLOAD_BOOTSTRAP
        ]))})</script>`));
    }
}
function writeFlightDataInstruction(controller, scriptStart, chunk) {
    let htmlInlinedData;
    if (typeof chunk === 'string') {
        htmlInlinedData = (0, _htmlescape.htmlEscapeJsonString)(JSON.stringify([
            INLINE_FLIGHT_PAYLOAD_DATA,
            chunk
        ]));
    } else {
        // The chunk cannot be embedded as a UTF-8 string in the script tag.
        // Instead let's inline it in base64.
        // Credits to Devon Govett (devongovett) for the technique.
        // https://github.com/devongovett/rsc-html-stream
        const base64 = btoa(String.fromCodePoint(...chunk));
        htmlInlinedData = (0, _htmlescape.htmlEscapeJsonString)(JSON.stringify([
            INLINE_FLIGHT_PAYLOAD_BINARY,
            base64
        ]));
    }
    controller.enqueue(encoder.encode(`${scriptStart}self.__next_f.push(${htmlInlinedData})</script>`));
}

//# sourceMappingURL=use-flight-response.js.map