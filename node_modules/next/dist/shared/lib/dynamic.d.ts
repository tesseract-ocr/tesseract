import React, { type JSX } from 'react';
type ComponentModule<P = {}> = {
    default: React.ComponentType<P>;
};
export declare type LoaderComponent<P = {}> = Promise<React.ComponentType<P> | ComponentModule<P>>;
export declare type Loader<P = {}> = (() => LoaderComponent<P>) | LoaderComponent<P>;
export type LoaderMap = {
    [module: string]: () => Loader<any>;
};
export type LoadableGeneratedOptions = {
    webpack?(): any;
    modules?(): LoaderMap;
};
export type DynamicOptionsLoadingProps = {
    error?: Error | null;
    isLoading?: boolean;
    pastDelay?: boolean;
    retry?: () => void;
    timedOut?: boolean;
};
export type DynamicOptions<P = {}> = LoadableGeneratedOptions & {
    loading?: (loadingProps: DynamicOptionsLoadingProps) => JSX.Element | null;
    loader?: Loader<P> | LoaderMap;
    loadableGenerated?: LoadableGeneratedOptions;
    ssr?: boolean;
};
export type LoadableOptions<P = {}> = DynamicOptions<P>;
export type LoadableFn<P = {}> = (opts: LoadableOptions<P>) => React.ComponentType<P>;
export type LoadableComponent<P = {}> = React.ComponentType<P>;
export declare function noSSR<P = {}>(LoadableInitializer: LoadableFn<P>, loadableOptions: DynamicOptions<P>): React.ComponentType<P>;
/**
 * This function lets you dynamically import a component.
 * It uses [React.lazy()](https://react.dev/reference/react/lazy) with [Suspense](https://react.dev/reference/react/Suspense) under the hood.
 *
 * Read more: [Next.js Docs: `next/dynamic`](https://nextjs.org/docs/app/building-your-application/optimizing/lazy-loading#nextdynamic)
 */
export default function dynamic<P = {}>(dynamicOptions: DynamicOptions<P> | Loader<P>, options?: DynamicOptions<P>): React.ComponentType<P>;
export {};
