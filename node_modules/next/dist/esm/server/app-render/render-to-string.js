import { streamToString } from "../stream-utils/node-web-streams-helper";
import { AppRenderSpan } from "../lib/trace/constants";
import { getTracer } from "../lib/trace/tracer";
export async function renderToString({ ReactDOMServer, element }) {
    return getTracer().trace(AppRenderSpan.renderToString, async ()=>{
        const renderStream = await ReactDOMServer.renderToReadableStream(element);
        await renderStream.allReady;
        return streamToString(renderStream);
    });
}

//# sourceMappingURL=render-to-string.js.map