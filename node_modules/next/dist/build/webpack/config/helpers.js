"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    loader: null,
    plugin: null,
    unshiftLoader: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    loader: function() {
        return loader;
    },
    plugin: function() {
        return plugin;
    },
    unshiftLoader: function() {
        return unshiftLoader;
    }
});
const _lodashcurry = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/lodash.curry"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const loader = (0, _lodashcurry.default)(function loader(rule, config) {
    var _config_module_rules;
    if (!config.module) {
        config.module = {
            rules: []
        };
    }
    if (rule.oneOf) {
        var _config_module_rules1;
        const existing = (_config_module_rules1 = config.module.rules) == null ? void 0 : _config_module_rules1.find((arrayRule)=>arrayRule && typeof arrayRule === 'object' && arrayRule.oneOf);
        if (existing && typeof existing === 'object') {
            existing.oneOf.push(...rule.oneOf);
            return config;
        }
    }
    (_config_module_rules = config.module.rules) == null ? void 0 : _config_module_rules.push(rule);
    return config;
});
const unshiftLoader = (0, _lodashcurry.default)(function unshiftLoader(rule, config) {
    var _config_module_rules;
    if (!config.module) {
        config.module = {
            rules: []
        };
    }
    if (rule.oneOf) {
        var _config_module_rules1;
        const existing = (_config_module_rules1 = config.module.rules) == null ? void 0 : _config_module_rules1.find((arrayRule)=>arrayRule && typeof arrayRule === 'object' && arrayRule.oneOf);
        if (existing && typeof existing === 'object') {
            var _existing_oneOf;
            (_existing_oneOf = existing.oneOf) == null ? void 0 : _existing_oneOf.unshift(...rule.oneOf);
            return config;
        }
    }
    (_config_module_rules = config.module.rules) == null ? void 0 : _config_module_rules.unshift(rule);
    return config;
});
const plugin = (0, _lodashcurry.default)(function plugin(p, config) {
    if (!config.plugins) {
        config.plugins = [];
    }
    config.plugins.push(p);
    return config;
});

//# sourceMappingURL=helpers.js.map