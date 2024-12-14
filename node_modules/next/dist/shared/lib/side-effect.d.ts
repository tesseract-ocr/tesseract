import type React from 'react';
import { type JSX } from 'react';
type State = JSX.Element[] | undefined;
export type SideEffectProps = {
    reduceComponentsToState: <T extends {}>(components: Array<React.ReactElement<any>>, props: T) => State;
    handleStateChange?: (state: State) => void;
    headManager: any;
    inAmpMode?: boolean;
    children: React.ReactNode;
};
export default function SideEffect(props: SideEffectProps): null;
export {};
