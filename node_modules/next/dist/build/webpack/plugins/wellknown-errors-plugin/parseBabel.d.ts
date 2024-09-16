import { SimpleWebpackError } from './simpleWebpackError';
export declare function getBabelError(fileName: string, err: Error & {
    code?: string | number;
    loc?: {
        line: number;
        column: number;
    };
}): SimpleWebpackError | false;
