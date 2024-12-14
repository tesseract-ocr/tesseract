import { type HMR_ACTION_TYPES } from '../../../../server/dev/hot-reloader-types';
type ActionCallback = (action: HMR_ACTION_TYPES) => void;
export declare function addMessageListener(callback: ActionCallback): void;
export declare function sendMessage(data: string): void;
export declare function connectHMR(options: {
    path: string;
    assetPrefix: string;
}): void;
export {};
