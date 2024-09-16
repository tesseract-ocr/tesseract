import type { BinaryLike } from 'crypto';
export type TelemetryEvent = {
    eventName: string;
    payload: object;
};
type RecordObject = {
    isFulfilled: boolean;
    isRejected: boolean;
    value?: any;
    reason?: any;
};
export declare class Telemetry {
    readonly sessionId: string;
    private conf;
    private distDir;
    private loadProjectId;
    private NEXT_TELEMETRY_DISABLED;
    private NEXT_TELEMETRY_DEBUG;
    private queue;
    constructor({ distDir }: {
        distDir: string;
    });
    private notify;
    get anonymousId(): string;
    get salt(): string;
    private get isDisabled();
    setEnabled: (_enabled: boolean) => string | null;
    get isEnabled(): boolean;
    oneWayHash: (payload: BinaryLike) => string;
    private getProjectId;
    record: (_events: TelemetryEvent | TelemetryEvent[], deferred?: boolean) => Promise<RecordObject>;
    flush: () => Promise<RecordObject[] | null>;
    flushDetached: (mode: 'dev', dir: string) => void;
    private submitRecord;
}
export {};
