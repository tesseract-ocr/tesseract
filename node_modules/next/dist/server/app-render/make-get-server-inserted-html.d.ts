import React from 'react';
export declare function makeGetServerInsertedHTML({ polyfills, renderServerInsertedHTML, serverCapturedErrors, basePath, }: {
    polyfills: JSX.IntrinsicElements['script'][];
    renderServerInsertedHTML: () => React.ReactNode;
    serverCapturedErrors: Error[];
    basePath: string;
}): () => Promise<string>;
