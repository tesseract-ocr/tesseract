interface UserAgent {
    isBot: boolean;
    ua: string;
    browser: {
        name?: string;
        version?: string;
    };
    device: {
        model?: string;
        type?: string;
        vendor?: string;
    };
    engine: {
        name?: string;
        version?: string;
    };
    os: {
        name?: string;
        version?: string;
    };
    cpu: {
        architecture?: string;
    };
}
export declare function isBot(input: string): boolean;
export declare function userAgentFromString(input: string | undefined): UserAgent;
export declare function userAgent({ headers }: {
    headers: Headers;
}): UserAgent;
export {};
