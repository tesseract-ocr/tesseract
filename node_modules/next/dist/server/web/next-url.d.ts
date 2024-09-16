/// <reference types="node" />
import type { OutgoingHttpHeaders } from 'http';
import type { DomainLocale, I18NConfig } from '../config-shared';
import type { I18NProvider } from '../future/helpers/i18n-provider';
interface Options {
    base?: string | URL;
    headers?: OutgoingHttpHeaders;
    forceLocale?: boolean;
    nextConfig?: {
        basePath?: string;
        i18n?: I18NConfig | null;
        trailingSlash?: boolean;
    };
    i18nProvider?: I18NProvider;
}
declare const Internal: unique symbol;
export declare class NextURL {
    private [Internal];
    constructor(input: string | URL, base?: string | URL, opts?: Options);
    constructor(input: string | URL, opts?: Options);
    private analyze;
    private formatPathname;
    private formatSearch;
    get buildId(): string | undefined;
    set buildId(buildId: string | undefined);
    get locale(): string;
    set locale(locale: string);
    get defaultLocale(): string | undefined;
    get domainLocale(): DomainLocale | undefined;
    get searchParams(): URLSearchParams;
    get host(): string;
    set host(value: string);
    get hostname(): string;
    set hostname(value: string);
    get port(): string;
    set port(value: string);
    get protocol(): string;
    set protocol(value: string);
    get href(): string;
    set href(url: string);
    get origin(): string;
    get pathname(): string;
    set pathname(value: string);
    get hash(): string;
    set hash(value: string);
    get search(): string;
    set search(value: string);
    get password(): string;
    set password(value: string);
    get username(): string;
    set username(value: string);
    get basePath(): string;
    set basePath(value: string);
    toString(): string;
    toJSON(): string;
    clone(): NextURL;
}
export {};
