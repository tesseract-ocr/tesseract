import { appendMutableCookies } from "../../../web/spec-extension/adapters/request-cookies";
export function handleRedirectResponse(url, mutableCookies, status) {
    const headers = new Headers({
        location: url
    });
    appendMutableCookies(headers, mutableCookies);
    return new Response(null, {
        status,
        headers
    });
}
export function handleBadRequestResponse() {
    return new Response(null, {
        status: 400
    });
}
export function handleNotFoundResponse() {
    return new Response(null, {
        status: 404
    });
}
export function handleMethodNotAllowedResponse() {
    return new Response(null, {
        status: 405
    });
}
export function handleInternalServerErrorResponse() {
    return new Response(null, {
        status: 500
    });
}

//# sourceMappingURL=response-handlers.js.map