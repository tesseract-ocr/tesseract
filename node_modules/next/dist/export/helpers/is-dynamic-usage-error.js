"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "isDynamicUsageError", {
    enumerable: true,
    get: function() {
        return isDynamicUsageError;
    }
});
const _hooksservercontext = require("../../client/components/hooks-server-context");
const _bailouttocsr = require("../../shared/lib/lazy-dynamic/bailout-to-csr");
const _isnextroutererror = require("../../client/components/is-next-router-error");
const _dynamicrendering = require("../../server/app-render/dynamic-rendering");
const isDynamicUsageError = (err)=>(0, _hooksservercontext.isDynamicServerError)(err) || (0, _bailouttocsr.isBailoutToCSRError)(err) || (0, _isnextroutererror.isNextRouterError)(err) || (0, _dynamicrendering.isDynamicPostpone)(err);

//# sourceMappingURL=is-dynamic-usage-error.js.map