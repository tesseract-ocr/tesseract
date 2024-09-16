import { SimpleWebpackError } from './simpleWebpackError';
import type { webpack } from 'next/dist/compiled/webpack/webpack';
export declare function getNotFoundError(compilation: webpack.Compilation, input: any, fileName: string, module: any): Promise<any>;
export declare function getImageError(compilation: any, input: any, err: Error): Promise<SimpleWebpackError | false>;
