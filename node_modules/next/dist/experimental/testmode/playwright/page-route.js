"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "handleRoute", {
    enumerable: true,
    get: function() {
        return handleRoute;
    }
});
function continueRoute(route, request, testHeaders) {
    return route.continue({
        headers: {
            ...request.headers(),
            ...testHeaders
        }
    });
}
async function handleRoute(route, page, testHeaders, fetchHandler) {
    const request = route.request();
    // Continue the navigation and non-fetch requests.
    if (request.isNavigationRequest() || request.resourceType() !== "fetch") {
        return continueRoute(route, request, testHeaders);
    }
    // Continue the local requests. The followup requests will be intercepted
    // on the server.
    const pageOrigin = new URL(page.url()).origin;
    const requestOrigin = new URL(request.url()).origin;
    if (pageOrigin === requestOrigin) {
        return continueRoute(route, request, testHeaders);
    }
    if (!fetchHandler) {
        return route.abort();
    }
    const postData = request.postDataBuffer();
    const fetchRequest = new Request(request.url(), {
        method: request.method(),
        headers: Object.fromEntries(Object.entries(request.headers()).filter(([name])=>!name.toLowerCase().startsWith("next-test-"))),
        body: postData ?? null
    });
    const proxyResponse = await fetchHandler(fetchRequest);
    if (!proxyResponse) {
        return route.abort();
    }
    if (proxyResponse === "abort") {
        return route.abort();
    }
    if (proxyResponse === "continue") {
        return continueRoute(route, request, testHeaders);
    }
    const { status, headers, body } = proxyResponse;
    return route.fulfill({
        status,
        headers: Object.fromEntries(headers),
        body: body ? Buffer.from(await proxyResponse.arrayBuffer()) : undefined
    });
}

//# sourceMappingURL=page-route.js.map