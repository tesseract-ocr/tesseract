import * as React from 'react';
type ErrorType = 'runtime' | 'build';
interface ReactDevOverlayProps {
    children?: React.ReactNode;
    preventDisplay?: ErrorType[];
    globalOverlay?: boolean;
}
export default function ReactDevOverlay({ children, preventDisplay, globalOverlay, }: ReactDevOverlayProps): import("react/jsx-runtime").JSX.Element;
export {};
