type AnonymousMeta = {
    systemPlatform: NodeJS.Platform;
    systemRelease: string;
    systemArchitecture: string;
    cpuCount: number;
    cpuModel: string | null;
    cpuSpeed: number | null;
    memoryInMb: number;
    isDocker: boolean;
    isNowDev: boolean;
    isWsl: boolean;
    isCI: boolean;
    ciName: string | null;
    nextVersion: string;
};
export declare function getAnonymousMeta(): AnonymousMeta;
export {};
