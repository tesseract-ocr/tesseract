import { startTransition, useCallback } from 'react';
import { ACTION_SERVER_ACTION } from './components/router-reducer/router-reducer-types';
let globalServerActionDispatcher = null;
export function useServerActionDispatcher(dispatch) {
    const serverActionDispatcher = useCallback((actionPayload)=>{
        startTransition(()=>{
            dispatch({
                ...actionPayload,
                type: ACTION_SERVER_ACTION
            });
        });
    }, [
        dispatch
    ]);
    globalServerActionDispatcher = serverActionDispatcher;
}
export async function callServer(actionId, actionArgs) {
    const actionDispatcher = globalServerActionDispatcher;
    if (!actionDispatcher) {
        throw new Error('Invariant: missing action dispatcher.');
    }
    return new Promise((resolve, reject)=>{
        actionDispatcher({
            actionId,
            actionArgs,
            resolve,
            reject
        });
    });
}

//# sourceMappingURL=app-call-server.js.map