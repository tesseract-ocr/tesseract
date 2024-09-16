import type { ViewportLayout } from './types/extra-types';
import type { Icons } from './types/metadata-types';
export declare const ViewportMetaKeys: {
    [k in keyof ViewportLayout]: string;
};
export declare const IconKeys: (keyof Icons)[];
