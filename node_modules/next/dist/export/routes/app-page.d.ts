import type { ExportRouteResult, FileWriter } from '../types';
import type { RenderOpts } from '../../server/app-render/types';
import type { NextParsedUrlQuery } from '../../server/request-meta';
import type { MockedRequest, MockedResponse } from '../../server/lib/mock-request';
export declare const enum ExportedAppPageFiles {
    HTML = "HTML",
    FLIGHT = "FLIGHT",
    PREFETCH_FLIGHT = "PREFETCH_FLIGHT",
    META = "META",
    POSTPONED = "POSTPONED"
}
export declare function exportAppPage(req: MockedRequest, res: MockedResponse, page: string, path: string, pathname: string, query: NextParsedUrlQuery, renderOpts: RenderOpts, htmlFilepath: string, debugOutput: boolean, isDynamicError: boolean, fileWriter: FileWriter): Promise<ExportRouteResult>;
