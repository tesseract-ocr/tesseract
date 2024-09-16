import type { StackFrame } from 'next/dist/compiled/stacktrace-parser';
import { type ErrorSourceType } from '../../../../../shared/lib/error-source';
export declare function getFilesystemFrame(frame: StackFrame): StackFrame;
export declare function getServerError(error: Error, type: ErrorSourceType): Error;
