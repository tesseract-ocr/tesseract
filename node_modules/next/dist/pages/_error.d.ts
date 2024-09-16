import React from 'react';
import type { NextPageContext } from '../shared/lib/utils';
export type ErrorProps = {
    statusCode: number;
    title?: string;
    withDarkMode?: boolean;
};
declare function _getInitialProps({ res, err, }: NextPageContext): Promise<ErrorProps> | ErrorProps;
/**
 * `Error` component used for handling errors.
 */
export default class Error<P = {}> extends React.Component<P & ErrorProps> {
    static displayName: string;
    static getInitialProps: typeof _getInitialProps;
    static origGetInitialProps: typeof _getInitialProps;
    render(): import("react/jsx-runtime").JSX.Element;
}
export {};
