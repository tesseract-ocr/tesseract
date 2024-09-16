import type { webpack } from 'next/dist/compiled/webpack/webpack';
import { SimpleWebpackError } from './simpleWebpackError';
export declare function getNextInvalidImportError(err: Error, module: any, compilation: webpack.Compilation, compiler: webpack.Compiler): SimpleWebpackError | false;
