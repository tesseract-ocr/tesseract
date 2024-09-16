import type { AdapterOptions } from '../../server/web/adapter';
import '../../server/web/globals';
export default function nHandler(opts: Omit<AdapterOptions, 'IncrementalCache' | 'page' | 'handler'>): Promise<import("../../server/web/types").FetchEventResult>;
