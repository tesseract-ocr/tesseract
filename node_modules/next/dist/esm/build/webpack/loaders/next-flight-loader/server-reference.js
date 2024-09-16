/* eslint-disable import/no-extraneous-dependencies */ import { registerServerReference as flightRegisterServerReference } from "react-server-dom-webpack/server.edge";
export function registerServerReference(id, action) {
    return flightRegisterServerReference(action, id, null);
}

//# sourceMappingURL=server-reference.js.map