import { getServerActionDispatcher } from "./components/app-router";
export async function callServer(actionId, actionArgs) {
    const actionDispatcher = getServerActionDispatcher();
    if (!actionDispatcher) {
        throw new Error("Invariant: missing action dispatcher.");
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