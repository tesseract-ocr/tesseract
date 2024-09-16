import { useEffect, useState } from "react";
import { createPortal } from "react-dom";
export const Portal = (param)=>{
    let { children, type } = param;
    const [portalNode, setPortalNode] = useState(null);
    useEffect(()=>{
        const element = document.createElement(type);
        document.body.appendChild(element);
        setPortalNode(element);
        return ()=>{
            document.body.removeChild(element);
        };
    }, [
        type
    ]);
    return portalNode ? /*#__PURE__*/ createPortal(children, portalNode) : null;
};

//# sourceMappingURL=index.js.map