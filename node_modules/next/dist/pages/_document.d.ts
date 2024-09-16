import React from 'react';
import type { ReactNode } from 'react';
import type { DocumentContext, DocumentInitialProps, DocumentProps } from '../shared/lib/utils';
import type { HtmlProps } from '../shared/lib/html-context.shared-runtime';
export type { DocumentContext, DocumentInitialProps, DocumentProps };
export type OriginProps = {
    nonce?: string;
    crossOrigin?: 'anonymous' | 'use-credentials' | '' | undefined;
    children?: React.ReactNode;
};
type DocumentFiles = {
    sharedFiles: readonly string[];
    pageFiles: readonly string[];
    allFiles: readonly string[];
};
type HeadHTMLProps = React.DetailedHTMLProps<React.HTMLAttributes<HTMLHeadElement>, HTMLHeadElement>;
type HeadProps = OriginProps & HeadHTMLProps;
export declare class Head extends React.Component<HeadProps> {
    static contextType: React.Context<HtmlProps | undefined>;
    context: HtmlProps;
    getCssLinks(files: DocumentFiles): JSX.Element[] | null;
    getPreloadDynamicChunks(): (import("react/jsx-runtime").JSX.Element | null)[];
    getPreloadMainLinks(files: DocumentFiles): JSX.Element[] | null;
    getBeforeInteractiveInlineScripts(): import("react/jsx-runtime").JSX.Element[];
    getDynamicChunks(files: DocumentFiles): (import("react/jsx-runtime").JSX.Element | null)[];
    getPreNextScripts(): import("react/jsx-runtime").JSX.Element;
    getScripts(files: DocumentFiles): import("react/jsx-runtime").JSX.Element[];
    getPolyfillScripts(): import("react/jsx-runtime").JSX.Element[];
    makeStylesheetInert(node: ReactNode[]): ReactNode[];
    render(): import("react/jsx-runtime").JSX.Element;
}
export declare class NextScript extends React.Component<OriginProps> {
    static contextType: React.Context<HtmlProps | undefined>;
    context: HtmlProps;
    getDynamicChunks(files: DocumentFiles): (import("react/jsx-runtime").JSX.Element | null)[];
    getPreNextScripts(): import("react/jsx-runtime").JSX.Element;
    getScripts(files: DocumentFiles): import("react/jsx-runtime").JSX.Element[];
    getPolyfillScripts(): import("react/jsx-runtime").JSX.Element[];
    static getInlineScriptSource(context: Readonly<HtmlProps>): string;
    render(): import("react/jsx-runtime").JSX.Element | null;
}
export declare function Html(props: React.DetailedHTMLProps<React.HtmlHTMLAttributes<HTMLHtmlElement>, HTMLHtmlElement>): import("react/jsx-runtime").JSX.Element;
export declare function Main(): import("react/jsx-runtime").JSX.Element;
/**
 * `Document` component handles the initial `document` markup and renders only on the server side.
 * Commonly used for implementing server side rendering for `css-in-js` libraries.
 */
export default class Document<P = {}> extends React.Component<DocumentProps & P> {
    /**
     * `getInitialProps` hook returns the context object with the addition of `renderPage`.
     * `renderPage` callback executes `React` rendering logic synchronously to support server-rendering wrappers
     */
    static getInitialProps(ctx: DocumentContext): Promise<DocumentInitialProps>;
    render(): import("react/jsx-runtime").JSX.Element;
}
