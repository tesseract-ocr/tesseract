import type { StackFrame } from 'next/dist/compiled/stacktrace-parser';
import type { VersionInfo } from '../../../server/dev/parse-version-info';
import type { SupportedErrorEvent } from './internal/container/Errors';
import type { ComponentStackFrame } from './internal/helpers/parse-component-stack';
import type { DebugInfo } from './types';
type FastRefreshState = 
/** No refresh in progress. */
{
    type: 'idle';
}
/** The refresh process has been triggered, but the new code has not been executed yet. */
 | {
    type: 'pending';
    errors: SupportedErrorEvent[];
};
export interface OverlayState {
    nextId: number;
    buildError: string | null;
    errors: SupportedErrorEvent[];
    refreshState: FastRefreshState;
    rootLayoutMissingTags: typeof window.__next_root_layout_missing_tags;
    versionInfo: VersionInfo;
    notFound: boolean;
    staticIndicator: boolean;
    debugInfo: DebugInfo | undefined;
}
export declare const ACTION_STATIC_INDICATOR = "static-indicator";
export declare const ACTION_BUILD_OK = "build-ok";
export declare const ACTION_BUILD_ERROR = "build-error";
export declare const ACTION_BEFORE_REFRESH = "before-fast-refresh";
export declare const ACTION_REFRESH = "fast-refresh";
export declare const ACTION_VERSION_INFO = "version-info";
export declare const ACTION_UNHANDLED_ERROR = "unhandled-error";
export declare const ACTION_UNHANDLED_REJECTION = "unhandled-rejection";
export declare const ACTION_DEBUG_INFO = "debug-info";
interface StaticIndicatorAction {
    type: typeof ACTION_STATIC_INDICATOR;
    staticIndicator: boolean;
}
interface BuildOkAction {
    type: typeof ACTION_BUILD_OK;
}
interface BuildErrorAction {
    type: typeof ACTION_BUILD_ERROR;
    message: string;
}
interface BeforeFastRefreshAction {
    type: typeof ACTION_BEFORE_REFRESH;
}
interface FastRefreshAction {
    type: typeof ACTION_REFRESH;
}
export interface UnhandledErrorAction {
    type: typeof ACTION_UNHANDLED_ERROR;
    reason: Error;
    frames: StackFrame[];
    componentStackFrames?: ComponentStackFrame[];
    warning?: [string, string, string];
}
export interface UnhandledRejectionAction {
    type: typeof ACTION_UNHANDLED_REJECTION;
    reason: Error;
    frames: StackFrame[];
}
export interface DebugInfoAction {
    type: typeof ACTION_DEBUG_INFO;
    debugInfo: any;
}
interface VersionInfoAction {
    type: typeof ACTION_VERSION_INFO;
    versionInfo: VersionInfo;
}
export type BusEvent = BuildOkAction | BuildErrorAction | BeforeFastRefreshAction | FastRefreshAction | UnhandledErrorAction | UnhandledRejectionAction | VersionInfoAction | StaticIndicatorAction | DebugInfoAction;
export declare const INITIAL_OVERLAY_STATE: OverlayState;
export declare function useErrorOverlayReducer(): [OverlayState, import("react").ActionDispatch<[action: BusEvent]>];
export declare const REACT_REFRESH_FULL_RELOAD_FROM_ERROR = "[Fast Refresh] performing full reload because your application had an unrecoverable error";
export {};
