"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "FlightRenderResult", {
    enumerable: true,
    get: function() {
        return FlightRenderResult;
    }
});
const _approuterheaders = require("../../client/components/app-router-headers");
const _renderresult = /*#__PURE__*/ _interop_require_default(require("../render-result"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
class FlightRenderResult extends _renderresult.default {
    constructor(response){
        super(response, {
            contentType: _approuterheaders.RSC_CONTENT_TYPE_HEADER,
            metadata: {}
        });
    }
}

//# sourceMappingURL=flight-render-result.js.map