"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    DYNAMIC_DATA: null,
    DYNAMIC_HTML: null,
    ServerRenderer: null,
    VoidRenderer: null,
    createStaticRenderer: null,
    getDynamicDataPostponedState: null,
    getDynamicHTMLPostponedState: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    DYNAMIC_DATA: function() {
        return DYNAMIC_DATA;
    },
    DYNAMIC_HTML: function() {
        return DYNAMIC_HTML;
    },
    ServerRenderer: function() {
        return ServerRenderer;
    },
    VoidRenderer: function() {
        return VoidRenderer;
    },
    createStaticRenderer: function() {
        return createStaticRenderer;
    },
    getDynamicDataPostponedState: function() {
        return getDynamicDataPostponedState;
    },
    getDynamicHTMLPostponedState: function() {
        return getDynamicHTMLPostponedState;
    }
});
class StaticRenderer {
    constructor(options){
        this.options = options;
        this.prerender = process.env.__NEXT_EXPERIMENTAL_REACT ? require("react-dom/static.edge").prerender : null;
    }
    async render(children) {
        const { prelude, postponed } = await this.prerender(children, this.options);
        return {
            stream: prelude,
            postponed
        };
    }
}
class StaticResumeRenderer {
    constructor(postponed, options){
        this.postponed = postponed;
        this.options = options;
        this.resume = require("react-dom/server.edge").resume;
    }
    async render(children) {
        const stream = await this.resume(children, this.postponed, this.options);
        return {
            stream,
            resumed: true
        };
    }
}
class ServerRenderer {
    constructor(options){
        this.options = options;
        this.renderToReadableStream = require("react-dom/server.edge").renderToReadableStream;
    }
    async render(children) {
        const stream = await this.renderToReadableStream(children, this.options);
        return {
            stream
        };
    }
}
class VoidRenderer {
    async render(_children) {
        return {
            stream: new ReadableStream({
                start (controller) {
                    // Close the stream immediately
                    controller.close();
                }
            }),
            resumed: false
        };
    }
}
const DYNAMIC_DATA = 1;
const DYNAMIC_HTML = 2;
function getDynamicHTMLPostponedState(data) {
    return [
        DYNAMIC_HTML,
        data
    ];
}
function getDynamicDataPostponedState() {
    return DYNAMIC_DATA;
}
function createStaticRenderer({ ppr, isStaticGeneration, postponed, streamOptions: { signal, onError, onPostpone, onHeaders, maxHeadersLength, nonce, bootstrapScripts, formState } }) {
    if (ppr) {
        if (isStaticGeneration) {
            // This is a Prerender
            return new StaticRenderer({
                signal,
                onError,
                onPostpone,
                // We want to capture headers because we may not end up with a shell
                // and being able to send headers is the next best thing
                onHeaders,
                maxHeadersLength,
                bootstrapScripts
            });
        } else {
            // This is a Resume
            if (postponed === DYNAMIC_DATA) {
                // The HTML was complete, we don't actually need to render anything
                return new VoidRenderer();
            } else if (postponed) {
                const reactPostponedState = postponed[1];
                // The HTML had dynamic holes and we need to resume it
                return new StaticResumeRenderer(reactPostponedState, {
                    signal,
                    onError,
                    onPostpone,
                    nonce
                });
            }
        }
    }
    if (isStaticGeneration) {
        // This is a static render (without PPR)
        return new ServerRenderer({
            signal,
            onError,
            // We don't pass onHeaders. In static builds we will either have no output
            // or the entire page. In either case preload headers aren't necessary and could
            // alter the prioritiy of relative loading of resources so we opt to keep them
            // as tags exclusively.
            nonce,
            bootstrapScripts,
            formState
        });
    }
    // This is a dynamic render (without PPR)
    return new ServerRenderer({
        signal,
        onError,
        // Static renders are streamed in realtime so sending headers early is
        // generally good because it will likely go out before the shell is ready.
        onHeaders,
        maxHeadersLength,
        nonce,
        bootstrapScripts,
        formState
    });
}

//# sourceMappingURL=static-renderer.js.map