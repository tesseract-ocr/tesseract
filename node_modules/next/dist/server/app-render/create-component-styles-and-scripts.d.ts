import React from 'react';
import type { AppRenderContext } from './app-render';
export declare function createComponentStylesAndScripts({ filePath, getComponent, injectedCSS, injectedJS, ctx, }: {
    filePath: string;
    getComponent: () => any;
    injectedCSS: Set<string>;
    injectedJS: Set<string>;
    ctx: AppRenderContext;
}): Promise<[React.ComponentType<any>, React.ReactNode, React.ReactNode]>;
