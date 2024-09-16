export interface VersionInfo {
    installed: string;
    staleness: 'fresh' | 'stale-patch' | 'stale-minor' | 'stale-major' | 'stale-prerelease' | 'newer-than-npm' | 'unknown';
    expected?: string;
}
export declare function parseVersionInfo(o: {
    installed: string;
    latest: string;
    canary: string;
}): VersionInfo;
