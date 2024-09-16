import type { webpack } from 'next/dist/compiled/webpack/webpack';
type SourceType = 'auto' | 'commonjs' | 'module';
export declare function getAssumedSourceType(mod: webpack.Module, sourceType: SourceType): SourceType;
export default function transformSource(this: any, source: string, sourceMap: any): void;
export {};
