"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return _default;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _mitt = /*#__PURE__*/ _interop_require_default._(require("../../shared/lib/mitt"));
class Span {
    end(endTime) {
        if (this.state.state === "ended") {
            throw new Error("Span has already ended");
        }
        this.state = {
            state: "ended",
            endTime: endTime != null ? endTime : Date.now()
        };
        this.onSpanEnd(this);
    }
    constructor(name, options, onSpanEnd){
        this.name = name;
        var _options_attributes;
        this.attributes = (_options_attributes = options.attributes) != null ? _options_attributes : {};
        var _options_startTime;
        this.startTime = (_options_startTime = options.startTime) != null ? _options_startTime : Date.now();
        this.onSpanEnd = onSpanEnd;
        this.state = {
            state: "inprogress"
        };
    }
}
class Tracer {
    startSpan(name, options) {
        return new Span(name, options, this.handleSpanEnd);
    }
    onSpanEnd(cb) {
        this._emitter.on("spanend", cb);
        return ()=>{
            this._emitter.off("spanend", cb);
        };
    }
    constructor(){
        this._emitter = (0, _mitt.default)();
        this.handleSpanEnd = (span)=>{
            this._emitter.emit("spanend", span);
        };
    }
}
const _default = new Tracer();

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=tracer.js.map