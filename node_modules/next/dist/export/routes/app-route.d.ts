import type { ExportRouteResult, FileWriter } from '../types';
import type AppRouteRouteModule from '../../server/route-modules/app-route/module';
import type { IncrementalCache } from '../../server/lib/incremental-cache';
import type { MockedRequest, MockedResponse } from '../../server/lib/mock-request';
import type { ExperimentalConfig } from '../../server/config-shared';
import type { Params } from '../../server/request/params';
export declare const enum ExportedAppRouteFiles {
    BODY = "BODY",
    META = "META"
}
export declare function exportAppRoute(req: MockedRequest, res: MockedResponse, params: Params | undefined, page: string, module: AppRouteRouteModule, incrementalCache: IncrementalCache | undefined, cacheLifeProfiles: undefined | {
    [profile: string]: import('../../server/use-cache/cache-life').CacheLife;
}, htmlFilepath: string, fileWriter: FileWriter, experimental: Required<Pick<ExperimentalConfig, 'dynamicIO' | 'authInterrupts'>>, buildId: string): Promise<ExportRouteResult>;
