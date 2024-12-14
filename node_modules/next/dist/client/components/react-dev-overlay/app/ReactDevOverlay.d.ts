import React from 'react';
import type { OverlayState } from '../shared';
import type { Dispatcher } from './hot-reloader-client';
interface ReactDevOverlayState {
    isReactError: boolean;
}
export default class ReactDevOverlay extends React.PureComponent<{
    state: OverlayState;
    dispatcher?: Dispatcher;
    children: React.ReactNode;
}, ReactDevOverlayState> {
    state: {
        isReactError: boolean;
    };
    static getDerivedStateFromError(error: Error): ReactDevOverlayState;
    render(): import("react/jsx-runtime").JSX.Element;
}
export {};
