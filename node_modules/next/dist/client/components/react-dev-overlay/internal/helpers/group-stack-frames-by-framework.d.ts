import type { SourcePackage } from '../../server/shared';
import type { OriginalStackFrame } from './stack-frame';
export type StackFramesGroup = {
    framework?: SourcePackage | null;
    stackFrames: OriginalStackFrame[];
};
/**
 * Group sequences of stack frames by framework.
 *
 * Given the following stack frames:
 * Error
 *   user code
 *   user code
 *   react
 *   react
 *   next
 *   next
 *   react
 *   react
 *
 * The grouped stack frames would be:
 * > user code
 * > react
 * > next
 * > react
 *
 */
export declare function groupStackFramesByFramework(stackFrames: OriginalStackFrame[]): StackFramesGroup[];
