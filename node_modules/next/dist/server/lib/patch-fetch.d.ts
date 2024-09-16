import type { StaticGenerationAsyncStorage, StaticGenerationStore } from '../../client/components/static-generation-async-storage.external';
import type * as ServerHooks from '../../client/components/hooks-server-context';
export declare function validateRevalidate(revalidateVal: unknown, pathname: string): undefined | number | false;
export declare function validateTags(tags: any[], description: string): string[];
export declare function addImplicitTags(staticGenerationStore: StaticGenerationStore): string[];
interface PatchableModule {
    serverHooks: typeof ServerHooks;
    staticGenerationAsyncStorage: StaticGenerationAsyncStorage;
}
export declare function patchFetch(options: PatchableModule): void;
export {};
