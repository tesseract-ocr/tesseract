import React from 'react';
import type { LoadableGeneratedOptions, DynamicOptionsLoadingProps, Loader, LoaderComponent } from './lazy-dynamic/types';
export { type LoadableGeneratedOptions, type DynamicOptionsLoadingProps, type Loader, type LoaderComponent, };
export type DynamicOptions<P = {}> = LoadableGeneratedOptions & {
    loading?: (loadingProps: DynamicOptionsLoadingProps) => JSX.Element | null;
    loader?: Loader<P>;
    loadableGenerated?: LoadableGeneratedOptions;
    modules?: string[];
    ssr?: boolean;
};
export type LoadableOptions<P = {}> = DynamicOptions<P>;
export type LoadableFn<P = {}> = (opts: LoadableOptions<P>) => React.ComponentType<P>;
export type LoadableComponent<P = {}> = React.ComponentType<P>;
export default function dynamic<P = {}>(dynamicOptions: DynamicOptions<P> | Loader<P>, options?: DynamicOptions<P>): React.ComponentType<P>;
