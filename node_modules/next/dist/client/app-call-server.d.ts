import { type ReducerActions } from './components/router-reducer/router-reducer-types';
export declare function useServerActionDispatcher(dispatch: React.Dispatch<ReducerActions>): void;
export declare function callServer(actionId: string, actionArgs: any[]): Promise<unknown>;
