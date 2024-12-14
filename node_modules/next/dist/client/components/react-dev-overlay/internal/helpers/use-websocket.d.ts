import type { TurbopackMsgToBrowser } from '../../../../../server/dev/hot-reloader-types';
export declare function useWebsocket(assetPrefix: string): import("react").RefObject<WebSocket | undefined>;
export declare function useSendMessage(webSocketRef: ReturnType<typeof useWebsocket>): (data: string) => void;
export declare function useTurbopack(sendMessage: ReturnType<typeof useSendMessage>, onUpdateError: (err: unknown) => void): (msg: TurbopackMsgToBrowser) => void;
export declare function useWebsocketPing(websocketRef: ReturnType<typeof useWebsocket>): void;
