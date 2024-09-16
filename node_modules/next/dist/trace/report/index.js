"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "reporter", {
    enumerable: true,
    get: function() {
        return reporter;
    }
});
const _totelemetry = /*#__PURE__*/ _interop_require_default(require("./to-telemetry"));
const _tojson = /*#__PURE__*/ _interop_require_default(require("./to-json"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
class MultiReporter {
    constructor(reporters){
        this.reporters = [];
        this.reporters = reporters;
    }
    async flushAll() {
        await Promise.all(this.reporters.map((reporter)=>reporter.flushAll()));
    }
    report(event) {
        this.reporters.forEach((reporter)=>reporter.report(event));
    }
}
const reporter = new MultiReporter([
    _tojson.default,
    _totelemetry.default
]);

//# sourceMappingURL=index.js.map