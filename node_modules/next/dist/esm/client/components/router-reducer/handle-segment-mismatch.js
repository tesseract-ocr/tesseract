import { handleExternalUrl } from './reducers/navigate-reducer';
/**
 * Handles the case where the client router attempted to patch the tree but, due to a mismatch, the patch failed.
 * This will perform an MPA navigation to return the router to a valid state.
 */ export function handleSegmentMismatch(state, action, treePatch) {
    if (process.env.NODE_ENV === 'development') {
        console.warn('Performing hard navigation because your application experienced an unrecoverable error. If this keeps occurring, please file a Next.js issue.\n\n' + 'Reason: Segment mismatch\n' + ("Last Action: " + action.type + "\n\n") + ("Current Tree: " + JSON.stringify(state.tree) + "\n\n") + ("Tree Patch Payload: " + JSON.stringify(treePatch)));
    }
    return handleExternalUrl(state, {}, state.canonicalUrl, true);
}

//# sourceMappingURL=handle-segment-mismatch.js.map