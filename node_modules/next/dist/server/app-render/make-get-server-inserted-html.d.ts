import React, { type JSX } from 'react';
import type { ClientTraceDataEntry } from '../lib/trace/tracer';
export declare function makeGetServerInsertedHTML({ polyfills, renderServerInsertedHTML, serverCapturedErrors, tracingMetadata, basePath, }: {
    polyfills: JSX.IntrinsicElements['script'][];
    renderServerInsertedHTML: () => React.ReactNode;
    tracingMetadata: ClientTraceDataEntry[] | undefined;
    serverCapturedErrors: Array<unknown>;
    basePath: string;
}): () => Promise<string>;
