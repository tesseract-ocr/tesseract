"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return AmpAttributePatcher;
    }
});
function AmpAttributePatcher() {
    return {
        visitor: {
            JSXOpeningElement (path) {
                const openingElement = path.node;
                const { name, attributes } = openingElement;
                if (!(name && name.type === 'JSXIdentifier')) {
                    return;
                }
                if (!name.name.startsWith('amp-')) {
                    return;
                }
                for (const attribute of attributes){
                    if (attribute.type !== 'JSXAttribute') {
                        continue;
                    }
                    if (attribute.name.name === 'className') {
                        attribute.name.name = 'class';
                    }
                }
            }
        }
    };
}

//# sourceMappingURL=amp-attributes.js.map