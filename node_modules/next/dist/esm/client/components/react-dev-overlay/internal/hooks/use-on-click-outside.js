import * as React from 'react';
export function useOnClickOutside(el, handler) {
    React.useEffect(()=>{
        if (el == null || handler == null) {
            return;
        }
        const listener = (e)=>{
            // Do nothing if clicking ref's element or descendent elements
            if (!el || el.contains(e.target)) {
                return;
            }
            handler(e);
        };
        const root = el.getRootNode();
        root.addEventListener('mousedown', listener);
        root.addEventListener('touchstart', listener, {
            passive: false
        });
        return function() {
            root.removeEventListener('mousedown', listener);
            root.removeEventListener('touchstart', listener);
        };
    }, [
        handler,
        el
    ]);
}

//# sourceMappingURL=use-on-click-outside.js.map