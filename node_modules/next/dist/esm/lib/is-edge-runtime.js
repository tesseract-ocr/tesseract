import { SERVER_RUNTIME } from './constants';
export function isEdgeRuntime(value) {
    return value === SERVER_RUNTIME.experimentalEdge || value === SERVER_RUNTIME.edge;
}

//# sourceMappingURL=is-edge-runtime.js.map