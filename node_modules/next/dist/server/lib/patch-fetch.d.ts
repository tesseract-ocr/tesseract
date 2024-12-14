import type { WorkAsyncStorage } from '../app-render/work-async-storage.external';
import type { WorkUnitAsyncStorage } from '../app-render/work-unit-async-storage.external';
type Fetcher = typeof fetch;
type PatchedFetcher = Fetcher & {
    readonly __nextPatched: true;
    readonly __nextGetStaticStore: () => WorkAsyncStorage;
    readonly _nextOriginalFetch: Fetcher;
};
export declare const NEXT_PATCH_SYMBOL: unique symbol;
export declare function validateRevalidate(revalidateVal: unknown, route: string): undefined | number;
export declare function validateTags(tags: any[], description: string): string[];
interface PatchableModule {
    workAsyncStorage: WorkAsyncStorage;
    workUnitAsyncStorage: WorkUnitAsyncStorage;
}
export declare function createPatchedFetcher(originFetch: Fetcher, { workAsyncStorage, workUnitAsyncStorage }: PatchableModule): PatchedFetcher;
export declare function patchFetch(options: PatchableModule): void;
export {};
