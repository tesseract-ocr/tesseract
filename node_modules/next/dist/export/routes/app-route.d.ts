import type { ExportRouteResult, FileWriter } from '../types';
import type { IncrementalCache } from '../../server/lib/incremental-cache';
import type { MockedRequest, MockedResponse } from '../../server/lib/mock-request';
export declare const enum ExportedAppRouteFiles {
    BODY = "BODY",
    META = "META"
}
export declare function exportAppRoute(req: MockedRequest, res: MockedResponse, params: {
    [key: string]: string | string[];
} | undefined, page: string, incrementalCache: IncrementalCache | undefined, distDir: string, htmlFilepath: string, fileWriter: FileWriter): Promise<ExportRouteResult>;
