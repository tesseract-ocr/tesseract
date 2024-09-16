import * as React from 'react';
import type { StackFrame } from 'next/dist/compiled/stacktrace-parser';
export type CodeFrameProps = {
    stackFrame: StackFrame;
    codeFrame: string;
};
export declare const CodeFrame: React.FC<CodeFrameProps>;
