import { BloomFilter } from '../shared/lib/bloom-filter';
import type { Redirect } from './load-custom-routes';
export declare function createClientRouterFilter(paths: string[], redirects: Redirect[], allowedErrorRate?: number): {
    staticFilter: ReturnType<BloomFilter['export']>;
    dynamicFilter: ReturnType<BloomFilter['export']>;
};
