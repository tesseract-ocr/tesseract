"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "renderToString", {
    enumerable: true,
    get: function() {
        return renderToString;
    }
});
const _nodewebstreamshelper = require("../stream-utils/node-web-streams-helper");
const _constants = require("../lib/trace/constants");
const _tracer = require("../lib/trace/tracer");
async function renderToString({ ReactDOMServer, element }) {
    return (0, _tracer.getTracer)().trace(_constants.AppRenderSpan.renderToString, async ()=>{
        const renderStream = await ReactDOMServer.renderToReadableStream(element);
        await renderStream.allReady;
        return (0, _nodewebstreamshelper.streamToString)(renderStream);
    });
}

//# sourceMappingURL=render-to-string.js.map