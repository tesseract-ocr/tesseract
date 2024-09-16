import '../build/polyfills/polyfill-module';
import type Router from '../shared/lib/router/router';
import type { MittEmitter } from '../shared/lib/mitt';
import type { NEXT_DATA } from '../shared/lib/utils';
declare global {
    interface Window {
        __NEXT_HYDRATED?: boolean;
        __NEXT_HYDRATED_CB?: () => void;
        __NEXT_DATA__: NEXT_DATA;
        __NEXT_P: any[];
    }
}
export declare const version: string | undefined;
export declare let router: Router;
export declare const emitter: MittEmitter<string>;
export declare function initialize(opts?: {
    devClient?: any;
}): Promise<{
    assetPrefix: string;
}>;
export declare function hydrate(opts?: {
    beforeRender?: () => Promise<void>;
}): Promise<void>;
