import React from 'react';
import type { AppContextType, AppInitialProps, AppPropsType, NextWebVitalsMetric, AppType } from '../shared/lib/utils';
import type { Router } from '../client/router';
export type { AppInitialProps, AppType };
export type { NextWebVitalsMetric };
export type AppContext = AppContextType<Router>;
export type AppProps<P = any> = AppPropsType<Router, P>;
/**
 * `App` component is used for initialize of pages. It allows for overwriting and full control of the `page` initialization.
 * This allows for keeping state between navigation, custom error handling, injecting additional data.
 */
declare function appGetInitialProps({ Component, ctx, }: AppContext): Promise<AppInitialProps>;
export default class App<P = any, CP = {}, S = {}> extends React.Component<P & AppProps<CP>, S> {
    static origGetInitialProps: typeof appGetInitialProps;
    static getInitialProps: typeof appGetInitialProps;
    render(): import("react/jsx-runtime").JSX.Element;
}
