import type { FlightSegmentPath } from '../../server/app-render/types';
import type { ErrorComponent } from './error-boundary';
import React from 'react';
/**
 * OuterLayoutRouter handles the current segment as well as <Offscreen> rendering of other segments.
 * It can be rendered next to each other with a different `parallelRouterKey`, allowing for Parallel routes.
 */
export default function OuterLayoutRouter({ parallelRouterKey, segmentPath, error, errorStyles, errorScripts, templateStyles, templateScripts, template, notFound, notFoundStyles, }: {
    parallelRouterKey: string;
    segmentPath: FlightSegmentPath;
    error: ErrorComponent | undefined;
    errorStyles: React.ReactNode | undefined;
    errorScripts: React.ReactNode | undefined;
    templateStyles: React.ReactNode | undefined;
    templateScripts: React.ReactNode | undefined;
    template: React.ReactNode;
    notFound: React.ReactNode | undefined;
    notFoundStyles: React.ReactNode | undefined;
}): import("react/jsx-runtime").JSX.Element;
