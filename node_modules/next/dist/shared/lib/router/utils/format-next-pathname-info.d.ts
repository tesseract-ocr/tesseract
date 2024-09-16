import type { NextPathnameInfo } from './get-next-pathname-info';
interface ExtendedInfo extends NextPathnameInfo {
    defaultLocale?: string;
    ignorePrefix?: boolean;
}
export declare function formatNextPathnameInfo(info: ExtendedInfo): string;
export {};
