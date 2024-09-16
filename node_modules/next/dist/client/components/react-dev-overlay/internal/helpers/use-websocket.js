"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    useSendMessage: null,
    useTurbopack: null,
    useWebsocket: null,
    useWebsocketPing: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    useSendMessage: function() {
        return useSendMessage;
    },
    useTurbopack: function() {
        return useTurbopack;
    },
    useWebsocket: function() {
        return useWebsocket;
    },
    useWebsocketPing: function() {
        return useWebsocketPing;
    }
});
const _react = require("react");
const _approutercontextsharedruntime = require("../../../../../shared/lib/app-router-context.shared-runtime");
const _getsocketurl = require("./get-socket-url");
function useWebsocket(assetPrefix) {
    const webSocketRef = (0, _react.useRef)();
    (0, _react.useEffect)(()=>{
        if (webSocketRef.current) {
            return;
        }
        const url = (0, _getsocketurl.getSocketUrl)(assetPrefix);
        webSocketRef.current = new window.WebSocket("" + url + "/_next/webpack-hmr");
    }, [
        assetPrefix
    ]);
    return webSocketRef;
}
function useSendMessage(webSocketRef) {
    const sendMessage = (0, _react.useCallback)((data)=>{
        const socket = webSocketRef.current;
        if (!socket || socket.readyState !== socket.OPEN) {
            return;
        }
        return socket.send(data);
    }, [
        webSocketRef
    ]);
    return sendMessage;
}
function useTurbopack(sendMessage, onUpdateError) {
    const turbopackState = (0, _react.useRef)({
        init: false,
        // Until the dynamic import resolves, queue any turbopack messages which will be replayed.
        queue: [],
        callback: undefined
    });
    const processTurbopackMessage = (0, _react.useCallback)((msg)=>{
        const { callback, queue } = turbopackState.current;
        if (callback) {
            callback(msg);
        } else {
            queue.push(msg);
        }
    }, []);
    (0, _react.useEffect)(()=>{
        const { current: initCurrent } = turbopackState;
        // TODO(WEB-1589): only install if `process.turbopack` set.
        if (initCurrent.init) {
            return;
        }
        initCurrent.init = true;
        import(// @ts-expect-error requires "moduleResolution": "node16" in tsconfig.json and not .ts extension
        "@vercel/turbopack-ecmascript-runtime/dev/client/hmr-client.ts").then((param)=>{
            let { connect } = param;
            const { current } = turbopackState;
            connect({
                addMessageListener (cb) {
                    current.callback = cb;
                    // Replay all Turbopack messages before we were able to establish the HMR client.
                    for (const msg of current.queue){
                        cb(msg);
                    }
                    current.queue = undefined;
                },
                sendMessage,
                onUpdateError
            });
        });
    }, [
        sendMessage,
        onUpdateError
    ]);
    return processTurbopackMessage;
}
function useWebsocketPing(websocketRef) {
    const sendMessage = useSendMessage(websocketRef);
    const { tree } = (0, _react.useContext)(_approutercontextsharedruntime.GlobalLayoutRouterContext);
    (0, _react.useEffect)(()=>{
        // Taken from on-demand-entries-client.js
        const interval = setInterval(()=>{
            sendMessage(JSON.stringify({
                event: "ping",
                tree,
                appDirRoute: true
            }));
        }, 2500);
        return ()=>clearInterval(interval);
    }, [
        tree,
        sendMessage
    ]);
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=use-websocket.js.map