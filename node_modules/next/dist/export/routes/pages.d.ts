import type { ExportRouteResult, FileWriter } from '../types';
import type { RenderOpts } from '../../server/render';
import type { LoadComponentsReturnType } from '../../server/load-components';
import type { NextParsedUrlQuery } from '../../server/request-meta';
import type { Params } from '../../server/request/params';
import type { MockedRequest, MockedResponse } from '../../server/lib/mock-request';
export declare const enum ExportedPagesFiles {
    HTML = "HTML",
    DATA = "DATA",
    AMP_HTML = "AMP_HTML",
    AMP_DATA = "AMP_PAGE_DATA"
}
/**
 * Renders & exports a page associated with the /pages directory
 */
export declare function exportPagesPage(req: MockedRequest, res: MockedResponse, path: string, page: string, query: NextParsedUrlQuery, params: Params | undefined, htmlFilepath: string, htmlFilename: string, ampPath: string, subFolders: boolean, outDir: string, ampValidatorPath: string | undefined, pagesDataDir: string, buildExport: boolean, isDynamic: boolean, hasOrigQueryValues: boolean, renderOpts: RenderOpts, components: LoadComponentsReturnType, fileWriter: FileWriter): Promise<ExportRouteResult | undefined>;
