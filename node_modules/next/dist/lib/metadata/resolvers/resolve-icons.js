"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    resolveIcon: null,
    resolveIcons: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    resolveIcon: function() {
        return resolveIcon;
    },
    resolveIcons: function() {
        return resolveIcons;
    }
});
const _utils = require("../generate/utils");
const _resolveurl = require("./resolve-url");
const _constants = require("../constants");
function resolveIcon(icon) {
    if ((0, _resolveurl.isStringOrURL)(icon)) return {
        url: icon
    };
    else if (Array.isArray(icon)) return icon;
    return icon;
}
const resolveIcons = (icons)=>{
    if (!icons) {
        return null;
    }
    const resolved = {
        icon: [],
        apple: []
    };
    if (Array.isArray(icons)) {
        resolved.icon = icons.map(resolveIcon).filter(Boolean);
    } else if ((0, _resolveurl.isStringOrURL)(icons)) {
        resolved.icon = [
            resolveIcon(icons)
        ];
    } else {
        for (const key of _constants.IconKeys){
            const values = (0, _utils.resolveAsArrayOrUndefined)(icons[key]);
            if (values) resolved[key] = values.map(resolveIcon);
        }
    }
    return resolved;
};

//# sourceMappingURL=resolve-icons.js.map