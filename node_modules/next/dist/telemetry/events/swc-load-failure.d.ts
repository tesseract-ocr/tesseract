export type EventSwcLoadFailure = {
    eventName: string;
    payload: {
        platform: string;
        arch: string;
        nodeVersion: string;
        nextVersion: string;
        wasm?: 'enabled' | 'fallback' | 'failed';
        glibcVersion?: string;
        installedSwcPackages?: string;
        nativeBindingsErrorCode?: string;
    };
};
export declare function eventSwcLoadFailure(event?: EventSwcLoadFailure['payload']): Promise<void>;
