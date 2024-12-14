"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "stringifyQuery", {
    enumerable: true,
    get: function() {
        return stringifyQuery;
    }
});
const _requestmeta = require("./request-meta");
const _querystring = require("querystring");
const stringifyQuery = (req, query)=>{
    const initialQuery = (0, _requestmeta.getRequestMeta)(req, 'initQuery') || {};
    const initialQueryValues = Object.values(initialQuery);
    return (0, _querystring.stringify)(query, undefined, undefined, {
        encodeURIComponent (value) {
            if (value in initialQuery || initialQueryValues.some((initialQueryVal)=>{
                // `value` always refers to a query value, even if it's nested in an array
                return Array.isArray(initialQueryVal) ? initialQueryVal.includes(value) : initialQueryVal === value;
            })) {
                // Encode keys and values from initial query
                return encodeURIComponent(value);
            }
            return value;
        }
    });
};

//# sourceMappingURL=server-route-utils.js.map