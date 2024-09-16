import type { SupportedErrorEvent } from '../container/Errors';
import type { OriginalStackFrame } from './stack-frame';
import type { ComponentStackFrame } from './parse-component-stack';
export type ReadyRuntimeError = {
    id: number;
    runtime: true;
    error: Error;
    frames: OriginalStackFrame[];
    componentStackFrames?: ComponentStackFrame[];
};
export declare function getErrorByType(ev: SupportedErrorEvent, isAppDir: boolean): Promise<ReadyRuntimeError>;
