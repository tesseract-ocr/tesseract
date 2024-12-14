declare const _default: (mode: "webpack" | "turbopack") => {
    subscribeToHmrEvent(handler: any): void;
    onUnrecoverableError(): void;
    addTurbopackMessageListener(cb: (msg: import("../../server/dev/hot-reloader-types").TurbopackMsgToBrowser) => void): void;
    sendTurbopackMessage(msg: string): void;
    handleUpdateError(err: unknown): void;
};
export default _default;
