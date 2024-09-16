"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getDeploymentIdQueryOrEmptyString", {
    enumerable: true,
    get: function() {
        return getDeploymentIdQueryOrEmptyString;
    }
});
function getDeploymentIdQueryOrEmptyString() {
    if (process.env.NEXT_DEPLOYMENT_ID) {
        return `?dpl=${process.env.NEXT_DEPLOYMENT_ID}`;
    }
    return "";
}

//# sourceMappingURL=deployment-id.js.map