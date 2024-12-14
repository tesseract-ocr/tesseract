import type { NextJsHotReloaderInterface } from './hot-reloader-types';
import { type ServerFields, type SetupOpts } from '../lib/router-utils/setup-dev-bundler';
export declare function createHotReloaderTurbopack(opts: SetupOpts, serverFields: ServerFields, distDir: string, resetFetch: () => void): Promise<NextJsHotReloaderInterface>;
