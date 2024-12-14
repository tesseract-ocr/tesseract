import type { StackFrame } from 'next/dist/compiled/stacktrace-parser';
import type { OriginalStackFrameResponse } from '../../server/shared';
export interface OriginalStackFrame extends OriginalStackFrameResponse {
    error: boolean;
    reason: string | null;
    external: boolean;
    ignored: boolean;
    sourceStackFrame: StackFrame;
}
export declare function getOriginalStackFrames(frames: StackFrame[], type: 'server' | 'edge-server' | null, isAppDir: boolean, errorMessage: string): Promise<OriginalStackFrame[]>;
export declare function getFrameSource(frame: StackFrame): string;
