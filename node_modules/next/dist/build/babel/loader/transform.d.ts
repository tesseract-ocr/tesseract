import type { Span } from '../../../trace';
import type { NextJsLoaderContext } from './types';
export default function transform(this: NextJsLoaderContext, source: string, inputSourceMap: object | null | undefined, loaderOptions: any, filename: string, target: string, parentSpan: Span): {
    code: string;
    map: {
        version: number;
        sources: string[];
        names: string[];
        sourceRoot?: string;
        sourcesContent?: string[];
        mappings: string;
        file: string;
    } | null;
};
