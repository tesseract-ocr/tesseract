import type { ExportRouteResult, FileWriter } from '../types';
import type { RenderOpts } from '../../server/app-render/types';
import type { NextParsedUrlQuery } from '../../server/request-meta';
import type { MockedRequest, MockedResponse } from '../../server/lib/mock-request';
import type { FallbackRouteParams } from '../../server/request/fallback-params';
import type { RequestLifecycleOpts } from '../../server/base-server';
export declare const enum ExportedAppPageFiles {
    HTML = "HTML",
    FLIGHT = "FLIGHT",
    PREFETCH_FLIGHT = "PREFETCH_FLIGHT",
    PREFETCH_FLIGHT_SEGMENT = "PREFETCH_FLIGHT_SEGMENT",
    META = "META",
    POSTPONED = "POSTPONED"
}
export declare function prospectiveRenderAppPage(req: MockedRequest, res: MockedResponse, page: string, pathname: string, query: NextParsedUrlQuery, fallbackRouteParams: FallbackRouteParams | null, partialRenderOpts: Omit<RenderOpts, keyof RequestLifecycleOpts>): Promise<undefined>;
/**
 * Renders & exports a page associated with the /app directory
 */
export declare function exportAppPage(req: MockedRequest, res: MockedResponse, page: string, path: string, pathname: string, query: NextParsedUrlQuery, fallbackRouteParams: FallbackRouteParams | null, partialRenderOpts: Omit<RenderOpts, keyof RequestLifecycleOpts>, htmlFilepath: string, debugOutput: boolean, isDynamicError: boolean, fileWriter: FileWriter): Promise<ExportRouteResult>;
