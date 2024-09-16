import { useCallback, useContext, useEffect, useRef } from "react";
import { GlobalLayoutRouterContext } from "../../../../../shared/lib/app-router-context.shared-runtime";
import { getSocketUrl } from "./get-socket-url";
export function useWebsocket(assetPrefix) {
    const webSocketRef = useRef();
    useEffect(()=>{
        if (webSocketRef.current) {
            return;
        }
        const url = getSocketUrl(assetPrefix);
        webSocketRef.current = new window.WebSocket("" + url + "/_next/webpack-hmr");
    }, [
        assetPrefix
    ]);
    return webSocketRef;
}
export function useSendMessage(webSocketRef) {
    const sendMessage = useCallback((data)=>{
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
export function useTurbopack(sendMessage, onUpdateError) {
    const turbopackState = useRef({
        init: false,
        // Until the dynamic import resolves, queue any turbopack messages which will be replayed.
        queue: [],
        callback: undefined
    });
    const processTurbopackMessage = useCallback((msg)=>{
        const { callback, queue } = turbopackState.current;
        if (callback) {
            callback(msg);
        } else {
            queue.push(msg);
        }
    }, []);
    useEffect(()=>{
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
export function useWebsocketPing(websocketRef) {
    const sendMessage = useSendMessage(websocketRef);
    const { tree } = useContext(GlobalLayoutRouterContext);
    useEffect(()=>{
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

//# sourceMappingURL=use-websocket.js.map