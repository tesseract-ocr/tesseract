"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "stripFlightHeaders", {
    enumerable: true,
    get: function() {
        return stripFlightHeaders;
    }
});
const _approuterheaders = require("../../client/components/app-router-headers");
function stripFlightHeaders(headers) {
    for (const [header] of _approuterheaders.FLIGHT_PARAMETERS){
        delete headers[header.toLowerCase()];
    }
}

//# sourceMappingURL=strip-flight-headers.js.map