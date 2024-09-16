/// <reference types="lodash" />
import type { webpack } from 'next/dist/compiled/webpack/webpack';
import type { ConfigurationContext } from '../../utils';
export declare const regexLikeCss: RegExp;
export declare function lazyPostCSS(rootDirectory: string, supportedBrowsers: string[] | undefined, disablePostcssPresetEnv: boolean | undefined): Promise<any>;
export declare const css: import("lodash").CurriedFunction2<ConfigurationContext, webpack.Configuration, Promise<webpack.Configuration>>;
