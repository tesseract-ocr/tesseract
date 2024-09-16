/// <reference types="node" />
import type { IncomingMessage, ServerResponse } from 'http';
import type { SizeLimit } from '../../../types';
import type { RequestStore } from '../../client/components/request-async-storage.external';
import type { AppRenderContext, GenerateFlight } from './app-render';
import type { AppPageModule } from '../../server/future/route-modules/app-page/module';
import RenderResult from '../render-result';
import type { StaticGenerationStore } from '../../client/components/static-generation-async-storage.external';
type ServerModuleMap = Record<string, {
    id: string;
    chunks: string[];
    name: string;
} | undefined>;
export declare function handleAction({ req, res, ComponentMod, serverModuleMap, generateFlight, staticGenerationStore, requestStore, serverActions, ctx, }: {
    req: IncomingMessage;
    res: ServerResponse;
    ComponentMod: AppPageModule;
    serverModuleMap: ServerModuleMap;
    generateFlight: GenerateFlight;
    staticGenerationStore: StaticGenerationStore;
    requestStore: RequestStore;
    serverActions?: {
        bodySizeLimit?: SizeLimit;
        allowedOrigins?: string[];
    };
    ctx: AppRenderContext;
}): Promise<undefined | {
    type: 'not-found';
} | {
    type: 'done';
    result: RenderResult | undefined;
    formState?: any;
}>;
export {};
