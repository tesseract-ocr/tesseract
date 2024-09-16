import type { WorkerRequestHandler } from '../../server/lib/types';
import type { NodeRequestHandler } from '../../server/next-server';
export declare function interceptTestApis(): () => void;
export declare function wrapRequestHandlerWorker(handler: WorkerRequestHandler): WorkerRequestHandler;
export declare function wrapRequestHandlerNode(handler: NodeRequestHandler): NodeRequestHandler;
