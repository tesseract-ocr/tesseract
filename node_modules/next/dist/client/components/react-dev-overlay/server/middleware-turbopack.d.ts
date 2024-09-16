/// <reference types="node" />
import type { IncomingMessage, ServerResponse } from 'http';
import { type OriginalStackFrameResponse } from './shared';
import type { StackFrame } from 'next/dist/compiled/stacktrace-parser';
import type { Project, TurbopackStackFrame } from '../../../../build/swc';
export declare function batchedTraceSource(project: Project, frame: TurbopackStackFrame): Promise<{
    frame: StackFrame;
    source: string | null;
} | undefined>;
export declare function createOriginalStackFrame(project: Project, frame: TurbopackStackFrame): Promise<OriginalStackFrameResponse | null>;
export declare function getOverlayMiddleware(project: Project): (req: IncomingMessage, res: ServerResponse) => Promise<void | ServerResponse<IncomingMessage>>;
