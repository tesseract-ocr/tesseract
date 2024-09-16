export type EventCliSessionStopped = {
    cliCommand: string;
    nextVersion: string;
    nodeVersion: string;
    turboFlag?: boolean | null;
    durationMilliseconds?: number | null;
    pagesDir?: boolean;
    appDir?: boolean;
};
export declare function eventCliSessionStopped(event: Omit<EventCliSessionStopped, 'nextVersion' | 'nodeVersion'>): {
    eventName: string;
    payload: EventCliSessionStopped;
}[];
