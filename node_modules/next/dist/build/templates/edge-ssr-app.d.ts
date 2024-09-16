import '../../server/web/globals';
import type { RequestData } from '../../server/web/types';
export declare const ComponentMod: any;
export default function nHandler(opts: {
    page: string;
    request: RequestData;
}): Promise<import("../../server/web/types").FetchEventResult>;
