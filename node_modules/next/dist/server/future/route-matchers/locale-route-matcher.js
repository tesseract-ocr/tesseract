"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "LocaleRouteMatcher", {
    enumerable: true,
    get: function() {
        return LocaleRouteMatcher;
    }
});
const _routematcher = require("./route-matcher");
class LocaleRouteMatcher extends _routematcher.RouteMatcher {
    /**
   * Identity returns the identity part of the matcher. This is used to compare
   * a unique matcher to another. This is also used when sorting dynamic routes,
   * so it must contain the pathname part as well.
   */ get identity() {
        var _this_definition_i18n;
        return `${this.definition.pathname}?__nextLocale=${(_this_definition_i18n = this.definition.i18n) == null ? void 0 : _this_definition_i18n.locale}`;
    }
    /**
   * Match will attempt to match the given pathname against this route while
   * also taking into account the locale information.
   *
   * @param pathname The pathname to match against.
   * @param options The options to use when matching.
   * @returns The match result, or `null` if there was no match.
   */ match(pathname, options) {
        var // If the options have a detected locale, then use that, otherwise use
        // the route's locale.
        _options_i18n, _this_definition_i18n;
        // This is like the parent `match` method but instead this injects the
        // additional `options` into the
        const result = this.test(pathname, options);
        if (!result) return null;
        return {
            definition: this.definition,
            params: result.params,
            detectedLocale: (options == null ? void 0 : (_options_i18n = options.i18n) == null ? void 0 : _options_i18n.detectedLocale) ?? ((_this_definition_i18n = this.definition.i18n) == null ? void 0 : _this_definition_i18n.locale)
        };
    }
    /**
   * Test will attempt to match the given pathname against this route while
   * also taking into account the locale information.
   *
   * @param pathname The pathname to match against.
   * @param options The options to use when matching.
   * @returns The match result, or `null` if there was no match.
   */ test(pathname, options) {
        // If this route has locale information and we have detected a locale, then
        // we need to compare the detected locale to the route's locale.
        if (this.definition.i18n && (options == null ? void 0 : options.i18n)) {
            // If we have detected a locale and it does not match this route's locale,
            // then this isn't a match!
            if (this.definition.i18n.locale && options.i18n.detectedLocale && this.definition.i18n.locale !== options.i18n.detectedLocale) {
                return null;
            }
            // Perform regular matching against the locale stripped pathname now, the
            // locale information matches!
            return super.test(options.i18n.pathname);
        }
        // If we don't have locale information, then we can just perform regular
        // matching.
        return super.test(pathname);
    }
}

//# sourceMappingURL=locale-route-matcher.js.map