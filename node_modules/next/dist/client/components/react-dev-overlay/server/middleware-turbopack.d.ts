import type { IncomingMessage, ServerResponse } from 'http';
import type { StackFrame } from 'next/dist/compiled/stacktrace-parser';
import type { Project, TurbopackStackFrame } from '../../../../build/swc/types';
type IgnorableStackFrame = StackFrame & {
    ignored: boolean;
};
export declare function batchedTraceSource(project: Project, frame: TurbopackStackFrame): Promise<{
    frame: IgnorableStackFrame;
    source: string | null;
} | undefined>;
export declare function getOverlayMiddleware(project: Project): (req: IncomingMessage, res: ServerResponse, next: () => void) => Promise<void>;
export declare function getSourceMapMiddleware(project: Project): (req: IncomingMessage, res: ServerResponse, next: () => void) => Promise<void>;
export {};
