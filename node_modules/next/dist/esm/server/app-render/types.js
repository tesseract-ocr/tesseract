import s from "next/dist/compiled/superstruct";
const dynamicParamTypesSchema = s.enums([
    "c",
    "ci",
    "oc",
    "d",
    "di"
]);
const segmentSchema = s.union([
    s.string(),
    s.tuple([
        s.string(),
        s.string(),
        dynamicParamTypesSchema
    ])
]);
// unfortunately the tuple is not understood well by Describe so we have to
// use any here. This does not have any impact on the runtime type since the validation
// does work correctly.
export const flightRouterStateSchema = s.tuple([
    segmentSchema,
    s.record(s.string(), s.lazy(()=>flightRouterStateSchema)),
    s.optional(s.nullable(s.string())),
    s.optional(s.nullable(s.union([
        s.literal("refetch"),
        s.literal("refresh")
    ]))),
    s.optional(s.boolean())
]);

//# sourceMappingURL=types.js.map