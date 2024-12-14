import { setAttributesFromProps } from './set-attributes-from-props';
function reactElementToDOM(param) {
    let { type, props } = param;
    const el = document.createElement(type);
    setAttributesFromProps(el, props);
    const { children, dangerouslySetInnerHTML } = props;
    if (dangerouslySetInnerHTML) {
        el.innerHTML = dangerouslySetInnerHTML.__html || '';
    } else if (children) {
        el.textContent = typeof children === 'string' ? children : Array.isArray(children) ? children.join('') : '';
    }
    return el;
}
/**
 * When a `nonce` is present on an element, browsers such as Chrome and Firefox strip it out of the
 * actual HTML attributes for security reasons *when the element is added to the document*. Thus,
 * given two equivalent elements that have nonces, `Element,isEqualNode()` will return false if one
 * of those elements gets added to the document. Although the `element.nonce` property will be the
 * same for both elements, the one that was added to the document will return an empty string for
 * its nonce HTML attribute value.
 *
 * This custom `isEqualNode()` function therefore removes the nonce value from the `newTag` before
 * comparing it to `oldTag`, restoring it afterwards.
 *
 * For more information, see:
 * https://bugs.chromium.org/p/chromium/issues/detail?id=1211471#c12
 */ export function isEqualNode(oldTag, newTag) {
    if (oldTag instanceof HTMLElement && newTag instanceof HTMLElement) {
        const nonce = newTag.getAttribute('nonce');
        // Only strip the nonce if `oldTag` has had it stripped. An element's nonce attribute will not
        // be stripped if there is no content security policy response header that includes a nonce.
        if (nonce && !oldTag.getAttribute('nonce')) {
            const cloneTag = newTag.cloneNode(true);
            cloneTag.setAttribute('nonce', '');
            cloneTag.nonce = nonce;
            return nonce === oldTag.nonce && oldTag.isEqualNode(cloneTag);
        }
    }
    return oldTag.isEqualNode(newTag);
}
let updateElements;
if (process.env.__NEXT_STRICT_NEXT_HEAD) {
    updateElements = (type, components)=>{
        const headEl = document.querySelector('head');
        if (!headEl) return;
        const oldTags = new Set(headEl.querySelectorAll("" + type + "[data-next-head]"));
        if (type === 'meta') {
            const metaCharset = headEl.querySelector('meta[charset]');
            if (metaCharset !== null) {
                oldTags.add(metaCharset);
            }
        }
        const newTags = [];
        for(let i = 0; i < components.length; i++){
            const component = components[i];
            const newTag = reactElementToDOM(component);
            newTag.setAttribute('data-next-head', '');
            let isNew = true;
            for (const oldTag of oldTags){
                if (isEqualNode(oldTag, newTag)) {
                    oldTags.delete(oldTag);
                    isNew = false;
                    break;
                }
            }
            if (isNew) {
                newTags.push(newTag);
            }
        }
        for (const oldTag of oldTags){
            var _oldTag_parentNode;
            (_oldTag_parentNode = oldTag.parentNode) == null ? void 0 : _oldTag_parentNode.removeChild(oldTag);
        }
        for (const newTag of newTags){
            // meta[charset] must be first element so special case
            if (newTag.tagName.toLowerCase() === 'meta' && newTag.getAttribute('charset') !== null) {
                headEl.prepend(newTag);
            }
            headEl.appendChild(newTag);
        }
    };
} else {
    updateElements = (type, components)=>{
        const headEl = document.getElementsByTagName('head')[0];
        const headCountEl = headEl.querySelector('meta[name=next-head-count]');
        if (process.env.NODE_ENV !== 'production') {
            if (!headCountEl) {
                console.error('Warning: next-head-count is missing. https://nextjs.org/docs/messages/next-head-count-missing');
                return;
            }
        }
        const headCount = Number(headCountEl.content);
        const oldTags = [];
        for(let i = 0, j = headCountEl.previousElementSibling; i < headCount; i++, j = (j == null ? void 0 : j.previousElementSibling) || null){
            var _j_tagName;
            if ((j == null ? void 0 : (_j_tagName = j.tagName) == null ? void 0 : _j_tagName.toLowerCase()) === type) {
                oldTags.push(j);
            }
        }
        const newTags = components.map(reactElementToDOM).filter((newTag)=>{
            for(let k = 0, len = oldTags.length; k < len; k++){
                const oldTag = oldTags[k];
                if (isEqualNode(oldTag, newTag)) {
                    oldTags.splice(k, 1);
                    return false;
                }
            }
            return true;
        });
        oldTags.forEach((t)=>{
            var _t_parentNode;
            return (_t_parentNode = t.parentNode) == null ? void 0 : _t_parentNode.removeChild(t);
        });
        newTags.forEach((t)=>headEl.insertBefore(t, headCountEl));
        headCountEl.content = (headCount - oldTags.length + newTags.length).toString();
    };
}
export default function initHeadManager() {
    return {
        mountedInstances: new Set(),
        updateHead: (head)=>{
            const tags = {};
            head.forEach((h)=>{
                if (// If the font tag is loaded only on client navigation
                // it won't be inlined. In this case revert to the original behavior
                h.type === 'link' && h.props['data-optimized-fonts']) {
                    if (document.querySelector('style[data-href="' + h.props['data-href'] + '"]')) {
                        return;
                    } else {
                        h.props.href = h.props['data-href'];
                        h.props['data-href'] = undefined;
                    }
                }
                const components = tags[h.type] || [];
                components.push(h);
                tags[h.type] = components;
            });
            const titleComponent = tags.title ? tags.title[0] : null;
            let title = '';
            if (titleComponent) {
                const { children } = titleComponent.props;
                title = typeof children === 'string' ? children : Array.isArray(children) ? children.join('') : '';
            }
            if (title !== document.title) document.title = title;
            [
                'meta',
                'base',
                'link',
                'style',
                'script'
            ].forEach((type)=>{
                updateElements(type, tags[type] || []);
            });
        }
    };
}

//# sourceMappingURL=head-manager.js.map