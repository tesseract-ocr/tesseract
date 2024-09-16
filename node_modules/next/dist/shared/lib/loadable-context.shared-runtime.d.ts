import React from 'react';
type CaptureFn = (moduleName: string) => void;
export declare const LoadableContext: React.Context<CaptureFn | null>;
export {};
