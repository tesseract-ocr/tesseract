import { jsx as _jsx } from "react/jsx-runtime";
import React from 'react';
import { nonNullable } from '../../non-nullable';
export function Meta({ name, property, content, media }) {
    if (typeof content !== 'undefined' && content !== null && content !== '') {
        return /*#__PURE__*/ _jsx("meta", {
            ...name ? {
                name
            } : {
                property
            },
            ...media ? {
                media
            } : undefined,
            content: typeof content === 'string' ? content : content.toString()
        });
    }
    return null;
}
export function MetaFilter(items) {
    const acc = [];
    for (const item of items){
        if (Array.isArray(item)) {
            acc.push(...item.filter(nonNullable));
        } else if (nonNullable(item)) {
            acc.push(item);
        }
    }
    return acc;
}
function camelToSnake(camelCaseStr) {
    return camelCaseStr.replace(/([A-Z])/g, function(match) {
        return '_' + match.toLowerCase();
    });
}
const aliasPropPrefixes = new Set([
    'og:image',
    'twitter:image',
    'og:video',
    'og:audio'
]);
function getMetaKey(prefix, key) {
    // Use `twitter:image` and `og:image` instead of `twitter:image:url` and `og:image:url`
    // to be more compatible as it's a more common format.
    // `og:video` & `og:audio` do not have a `:url` suffix alias
    if (aliasPropPrefixes.has(prefix) && key === 'url') {
        return prefix;
    }
    if (prefix.startsWith('og:') || prefix.startsWith('twitter:')) {
        key = camelToSnake(key);
    }
    return prefix + ':' + key;
}
function ExtendMeta({ content, namePrefix, propertyPrefix }) {
    if (!content) return null;
    return MetaFilter(Object.entries(content).map(([k, v])=>{
        return typeof v === 'undefined' ? null : Meta({
            ...propertyPrefix && {
                property: getMetaKey(propertyPrefix, k)
            },
            ...namePrefix && {
                name: getMetaKey(namePrefix, k)
            },
            content: typeof v === 'string' ? v : v == null ? void 0 : v.toString()
        });
    }));
}
export function MultiMeta({ propertyPrefix, namePrefix, contents }) {
    if (typeof contents === 'undefined' || contents === null) {
        return null;
    }
    return MetaFilter(contents.map((content)=>{
        if (typeof content === 'string' || typeof content === 'number' || content instanceof URL) {
            return Meta({
                ...propertyPrefix ? {
                    property: propertyPrefix
                } : {
                    name: namePrefix
                },
                content
            });
        } else {
            return ExtendMeta({
                namePrefix,
                propertyPrefix,
                content
            });
        }
    }));
}

//# sourceMappingURL=meta.js.map