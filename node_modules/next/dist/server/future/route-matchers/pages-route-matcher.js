"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    PagesLocaleRouteMatcher: null,
    PagesRouteMatcher: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    PagesLocaleRouteMatcher: function() {
        return PagesLocaleRouteMatcher;
    },
    PagesRouteMatcher: function() {
        return PagesRouteMatcher;
    }
});
const _localeroutematcher = require("./locale-route-matcher");
const _routematcher = require("./route-matcher");
class PagesRouteMatcher extends _routematcher.RouteMatcher {
}
class PagesLocaleRouteMatcher extends _localeroutematcher.LocaleRouteMatcher {
}

//# sourceMappingURL=pages-route-matcher.js.map