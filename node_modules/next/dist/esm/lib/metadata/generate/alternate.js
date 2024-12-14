import { jsx as _jsx } from "react/jsx-runtime";
import React from 'react';
import { MetaFilter } from './meta';
function AlternateLink({ descriptor, ...props }) {
    if (!descriptor.url) return null;
    return /*#__PURE__*/ _jsx("link", {
        ...props,
        ...descriptor.title && {
            title: descriptor.title
        },
        href: descriptor.url.toString()
    });
}
export function AlternatesMetadata({ alternates }) {
    if (!alternates) return null;
    const { canonical, languages, media, types } = alternates;
    return MetaFilter([
        canonical ? AlternateLink({
            rel: 'canonical',
            descriptor: canonical
        }) : null,
        languages ? Object.entries(languages).flatMap(([locale, descriptors])=>descriptors == null ? void 0 : descriptors.map((descriptor)=>AlternateLink({
                    rel: 'alternate',
                    hrefLang: locale,
                    descriptor
                }))) : null,
        media ? Object.entries(media).flatMap(([mediaName, descriptors])=>descriptors == null ? void 0 : descriptors.map((descriptor)=>AlternateLink({
                    rel: 'alternate',
                    media: mediaName,
                    descriptor
                }))) : null,
        types ? Object.entries(types).flatMap(([type, descriptors])=>descriptors == null ? void 0 : descriptors.map((descriptor)=>AlternateLink({
                    rel: 'alternate',
                    type,
                    descriptor
                }))) : null
    ]);
}

//# sourceMappingURL=alternate.js.map