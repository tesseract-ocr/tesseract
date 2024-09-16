import type { PluginItem } from 'next/dist/compiled/babel/core';
type StyledJsxPlugin = [string, any] | string;
type StyledJsxBabelOptions = {
    plugins?: StyledJsxPlugin[];
    styleModule?: string;
    'babel-test'?: boolean;
} | undefined;
type NextBabelPresetOptions = {
    'preset-env'?: any;
    'preset-react'?: any;
    'class-properties'?: any;
    'transform-runtime'?: any;
    'styled-jsx'?: StyledJsxBabelOptions;
    'preset-typescript'?: any;
};
type BabelPreset = {
    presets?: PluginItem[] | null;
    plugins?: PluginItem[] | null;
    sourceType?: 'script' | 'module' | 'unambiguous';
    overrides?: Array<{
        test: RegExp;
    } & Omit<BabelPreset, 'overrides'>>;
};
declare const _default: (api: any, options?: NextBabelPresetOptions) => BabelPreset;
export default _default;
