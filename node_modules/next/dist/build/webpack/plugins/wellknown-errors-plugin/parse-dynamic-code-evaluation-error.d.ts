import type { webpack } from 'next/dist/compiled/webpack/webpack';
import { SimpleWebpackError } from './simpleWebpackError';
export declare function getDynamicCodeEvaluationError(message: string, module: webpack.NormalModule, compilation: webpack.Compilation, compiler: webpack.Compiler): SimpleWebpackError;
