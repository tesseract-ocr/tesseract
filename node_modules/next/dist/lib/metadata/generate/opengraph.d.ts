import type { ResolvedMetadata } from '../types/metadata-interface';
export declare function OpenGraphMetadata({ openGraph, }: {
    openGraph: ResolvedMetadata['openGraph'];
}): NonNullable<import("react").ReactElement<unknown, string | import("react").JSXElementConstructor<any>> | NonNullable<import("react").ReactElement<unknown, string | import("react").JSXElementConstructor<any>> | import("react").ReactElement<unknown, string | import("react").JSXElementConstructor<any>>[]>[]>[] | null;
export declare function TwitterMetadata({ twitter, }: {
    twitter: ResolvedMetadata['twitter'];
}): NonNullable<import("react").ReactElement<unknown, string | import("react").JSXElementConstructor<any>> | (import("react").ReactElement<unknown, string | import("react").JSXElementConstructor<any>> | null)[] | NonNullable<import("react").ReactElement<unknown, string | import("react").JSXElementConstructor<any>> | import("react").ReactElement<unknown, string | import("react").JSXElementConstructor<any>>[]>[]>[] | null;
export declare function AppLinksMeta({ appLinks, }: {
    appLinks: ResolvedMetadata['appLinks'];
}): NonNullable<import("react").ReactElement<unknown, string | import("react").JSXElementConstructor<any>> | import("react").ReactElement<unknown, string | import("react").JSXElementConstructor<any>>[]>[][] | null;
