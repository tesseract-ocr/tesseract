/**
 * Converts the query into params.
 *
 * @param query the query to convert to params
 * @returns the params
 */ export function parsedUrlQueryToParams(query) {
    const params = {};
    for (const [key, value] of Object.entries(query)){
        if (typeof value === "undefined") continue;
        params[key] = value;
    }
    return params;
}

//# sourceMappingURL=parsed-url-query-to-params.js.map