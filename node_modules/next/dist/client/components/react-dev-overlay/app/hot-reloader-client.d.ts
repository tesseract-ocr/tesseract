import type { ReactNode } from 'react';
export default function HotReload({ assetPrefix, children, }: {
    assetPrefix: string;
    children?: ReactNode;
}): import("react/jsx-runtime").JSX.Element;
