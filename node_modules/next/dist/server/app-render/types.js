"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "flightRouterStateSchema", {
    enumerable: true,
    get: function() {
        return flightRouterStateSchema;
    }
});
const _superstruct = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/superstruct"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const dynamicParamTypesSchema = _superstruct.default.enums([
    'c',
    'ci',
    'oc',
    'd',
    'di'
]);
const segmentSchema = _superstruct.default.union([
    _superstruct.default.string(),
    _superstruct.default.tuple([
        _superstruct.default.string(),
        _superstruct.default.string(),
        dynamicParamTypesSchema
    ])
]);
const flightRouterStateSchema = _superstruct.default.tuple([
    segmentSchema,
    _superstruct.default.record(_superstruct.default.string(), _superstruct.default.lazy(()=>flightRouterStateSchema)),
    _superstruct.default.optional(_superstruct.default.nullable(_superstruct.default.string())),
    _superstruct.default.optional(_superstruct.default.nullable(_superstruct.default.union([
        _superstruct.default.literal('refetch'),
        _superstruct.default.literal('refresh')
    ]))),
    _superstruct.default.optional(_superstruct.default.boolean())
]);

//# sourceMappingURL=types.js.map