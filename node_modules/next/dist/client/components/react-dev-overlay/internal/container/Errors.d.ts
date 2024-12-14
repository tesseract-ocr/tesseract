import { type UnhandledErrorAction, type UnhandledRejectionAction } from '../../shared';
import type { DebugInfo } from '../../types';
import type { VersionInfo } from '../../../../../server/dev/parse-version-info';
export type SupportedErrorEvent = {
    id: number;
    event: UnhandledErrorAction | UnhandledRejectionAction;
};
export type ErrorsProps = {
    isAppDir: boolean;
    errors: SupportedErrorEvent[];
    initialDisplayState: DisplayState;
    versionInfo?: VersionInfo;
    hasStaticIndicator?: boolean;
    debugInfo?: DebugInfo;
};
type DisplayState = 'minimized' | 'fullscreen' | 'hidden';
export declare function Errors({ isAppDir, errors, initialDisplayState, versionInfo, hasStaticIndicator, debugInfo, }: ErrorsProps): import("react/jsx-runtime").JSX.Element | null;
export declare const styles: string;
export {};
