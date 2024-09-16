import type { webpack } from 'next/dist/compiled/webpack/webpack';
import { SimpleWebpackError } from './simpleWebpackError';
export declare function getNextAppLoaderError(err: Error, module: any, compiler: webpack.Compiler): SimpleWebpackError | false;
