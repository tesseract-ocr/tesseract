export function isGroupSegment(segment) {
    // Use array[0] for performant purpose
    return segment[0] === "(" && segment.endsWith(")");
}
export const PAGE_SEGMENT_KEY = "__PAGE__";
export const DEFAULT_SEGMENT_KEY = "__DEFAULT__";

//# sourceMappingURL=segment.js.map