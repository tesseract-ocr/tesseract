import type { ReactNode } from 'react';
import type { VersionInfo } from '../../../../server/dev/parse-version-info';
import type { DebugInfo } from '../types';
export interface Dispatcher {
    onBuildOk(): void;
    onBuildError(message: string): void;
    onVersionInfo(versionInfo: VersionInfo): void;
    onDebugInfo(debugInfo: DebugInfo): void;
    onBeforeRefresh(): void;
    onRefresh(): void;
    onStaticIndicator(status: boolean): void;
}
export declare function waitForWebpackRuntimeHotUpdate(): Promise<void>;
export default function HotReload({ assetPrefix, children, }: {
    assetPrefix: string;
    children?: ReactNode;
}): import("react/jsx-runtime").JSX.Element;
