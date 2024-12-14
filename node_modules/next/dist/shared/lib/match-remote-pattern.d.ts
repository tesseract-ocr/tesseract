import type { RemotePattern } from './image-config';
export declare function matchRemotePattern(pattern: RemotePattern, url: URL): boolean;
export declare function hasRemoteMatch(domains: string[], remotePatterns: RemotePattern[], url: URL): boolean;
