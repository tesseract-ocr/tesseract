import type { StackFrame } from 'next/dist/compiled/stacktrace-parser';
import { type OriginalStackFrameResponse } from './shared';
export { getServerError } from '../internal/helpers/nodeStackFrames';
export { parseStack } from '../internal/helpers/parseStack';
import type { IncomingMessage, ServerResponse } from 'http';
import type webpack from 'webpack';
type Source = {
    map: () => any;
} | null;
export declare function createOriginalStackFrame({ source, moduleId, modulePath, rootDirectory, frame, errorMessage, compilation, }: {
    source: any;
    moduleId?: string;
    modulePath?: string;
    rootDirectory: string;
    frame: StackFrame;
    errorMessage?: string;
    compilation?: webpack.Compilation;
}): Promise<OriginalStackFrameResponse | null>;
export declare function getSourceById(isFile: boolean, id: string, compilation?: webpack.Compilation): Promise<Source>;
export declare function getOverlayMiddleware(options: {
    rootDirectory: string;
    stats(): webpack.Stats | null;
    serverStats(): webpack.Stats | null;
    edgeServerStats(): webpack.Stats | null;
}): (req: IncomingMessage, res: ServerResponse, next: Function) => Promise<any>;
