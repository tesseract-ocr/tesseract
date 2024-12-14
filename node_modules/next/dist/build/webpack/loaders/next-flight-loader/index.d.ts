import type { webpack } from 'next/dist/compiled/webpack/webpack';
import type { javascript, LoaderContext } from 'next/dist/compiled/webpack/webpack';
type SourceType = javascript.JavascriptParser['sourceType'] | 'commonjs';
export declare function getAssumedSourceType(mod: webpack.Module, sourceType: SourceType): SourceType;
export default function transformSource(this: LoaderContext<undefined>, source: string, sourceMap: any): void;
export {};
