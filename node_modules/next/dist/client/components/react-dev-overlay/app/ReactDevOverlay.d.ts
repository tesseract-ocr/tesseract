import * as React from 'react';
import { type OverlayState } from '../shared';
import type { SupportedErrorEvent } from '../internal/container/Errors';
interface ReactDevOverlayState {
    reactError: SupportedErrorEvent | null;
}
export default class ReactDevOverlay extends React.PureComponent<{
    state: OverlayState;
    children: React.ReactNode;
    onReactError: (error: Error) => void;
}, ReactDevOverlayState> {
    state: {
        reactError: null;
    };
    static getDerivedStateFromError(error: Error): ReactDevOverlayState;
    componentDidCatch(componentErr: Error): void;
    render(): import("react/jsx-runtime").JSX.Element;
}
export {};
