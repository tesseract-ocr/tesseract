import type { SizeLimit } from '../../types';
import type { RequestStore } from '../app-render/work-unit-async-storage.external';
import type { AppRenderContext, GenerateFlight } from './app-render';
import type { AppPageModule } from '../route-modules/app-page/module';
import type { BaseNextRequest, BaseNextResponse } from '../base-http';
import RenderResult from '../render-result';
import type { WorkStore } from '../app-render/work-async-storage.external';
type ServerModuleMap = Record<string, {
    id: string;
    chunks: string[];
    name: string;
}>;
type ServerActionsConfig = {
    bodySizeLimit?: SizeLimit;
    allowedOrigins?: string[];
};
export declare function handleAction({ req, res, ComponentMod, serverModuleMap, generateFlight, workStore, requestStore, serverActions, ctx, }: {
    req: BaseNextRequest;
    res: BaseNextResponse;
    ComponentMod: AppPageModule;
    serverModuleMap: ServerModuleMap;
    generateFlight: GenerateFlight;
    workStore: WorkStore;
    requestStore: RequestStore;
    serverActions?: ServerActionsConfig;
    ctx: AppRenderContext;
}): Promise<undefined | {
    type: 'not-found';
} | {
    type: 'done';
    result: RenderResult | undefined;
    formState?: any;
}>;
export {};
