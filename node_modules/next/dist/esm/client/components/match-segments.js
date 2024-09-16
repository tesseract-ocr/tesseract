import { getSegmentParam } from "../../server/app-render/get-segment-param";
export const matchSegment = (existingSegment, segment)=>{
    // segment is either Array or string
    if (typeof existingSegment === "string") {
        if (typeof segment === "string") {
            // Common case: segment is just a string
            return existingSegment === segment;
        }
        return false;
    }
    if (typeof segment === "string") {
        return false;
    }
    return existingSegment[0] === segment[0] && existingSegment[1] === segment[1];
};
/*
 * This function is used to determine if an existing segment can be overridden by the incoming segment.
 */ export const canSegmentBeOverridden = (existingSegment, segment)=>{
    var _getSegmentParam;
    if (Array.isArray(existingSegment) || !Array.isArray(segment)) {
        return false;
    }
    return ((_getSegmentParam = getSegmentParam(existingSegment)) == null ? void 0 : _getSegmentParam.param) === segment[0];
};

//# sourceMappingURL=match-segments.js.map