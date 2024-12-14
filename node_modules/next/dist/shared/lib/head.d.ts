import React, { type JSX } from 'react';
export declare function defaultHead(inAmpMode?: boolean): JSX.Element[];
/**
 * This component injects elements to `<head>` of your page.
 * To avoid duplicated `tags` in `<head>` you can use the `key` property, which will make sure every tag is only rendered once.
 */
declare function Head({ children }: {
    children: React.ReactNode;
}): import("react/jsx-runtime").JSX.Element;
export default Head;
