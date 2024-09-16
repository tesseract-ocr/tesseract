import type { ResponseCookies } from '../../../web/spec-extension/cookies';
export declare function handleRedirectResponse(url: string, mutableCookies: ResponseCookies, status: number): Response;
export declare function handleBadRequestResponse(): Response;
export declare function handleNotFoundResponse(): Response;
export declare function handleMethodNotAllowedResponse(): Response;
export declare function handleInternalServerErrorResponse(): Response;
