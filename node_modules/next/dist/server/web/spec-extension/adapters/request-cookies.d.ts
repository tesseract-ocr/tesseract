import { RequestCookies } from '../cookies';
import { ResponseCookies } from '../cookies';
import { type RequestStore } from '../../../app-render/work-unit-async-storage.external';
export type { ResponseCookies };
export type ReadonlyRequestCookies = Omit<RequestCookies, 'set' | 'clear' | 'delete'> & Pick<ResponseCookies, 'set' | 'delete'>;
export declare class RequestCookiesAdapter {
    static seal(cookies: RequestCookies): ReadonlyRequestCookies;
}
export declare function getModifiedCookieValues(cookies: ResponseCookies): ResponseCookie[];
export declare function appendMutableCookies(headers: Headers, mutableCookies: ResponseCookies): boolean;
type ResponseCookie = NonNullable<ReturnType<InstanceType<typeof ResponseCookies>['get']>>;
export declare class MutableRequestCookiesAdapter {
    static wrap(cookies: RequestCookies, onUpdateCookies?: (cookies: string[]) => void): ResponseCookies;
}
export declare function wrapWithMutableAccessCheck(responseCookies: ResponseCookies): ResponseCookies;
export declare function areCookiesMutableInCurrentPhase(requestStore: RequestStore): boolean;
export declare function responseCookiesToRequestCookies(responseCookies: ResponseCookies): RequestCookies;
