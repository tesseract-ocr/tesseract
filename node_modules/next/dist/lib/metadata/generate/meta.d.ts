import React from 'react';
export declare function Meta({ name, property, content, media, }: {
    name?: string;
    property?: string;
    media?: string;
    content: string | number | URL | null | undefined;
}): React.ReactElement | null;
export declare function MetaFilter<T extends {} | {}[]>(items: (T | null)[]): NonNullable<T>[];
type ExtendMetaContent = Record<string, undefined | string | URL | number | boolean | null | undefined>;
type MultiMetaContent = (ExtendMetaContent | string | URL | number)[] | null | undefined;
export declare function MultiMeta({ propertyPrefix, namePrefix, contents, }: {
    propertyPrefix?: string;
    namePrefix?: string;
    contents?: MultiMetaContent | null;
}): NonNullable<React.ReactElement<any, string | React.JSXElementConstructor<any>> | React.ReactElement<any, string | React.JSXElementConstructor<any>>[]>[] | null;
export {};
