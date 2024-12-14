import type { ActionManifest } from '../../build/webpack/plugins/flight-client-entry-plugin';
export declare function createServerModuleMap({ serverActionsManifest, }: {
    serverActionsManifest: ActionManifest;
}): {};
/**
 * Checks if the requested action has a worker for the current page.
 * If not, it returns the first worker that has a handler for the action.
 */
export declare function selectWorkerForForwarding(actionId: string, pageName: string, serverActionsManifest: ActionManifest): string | undefined;
