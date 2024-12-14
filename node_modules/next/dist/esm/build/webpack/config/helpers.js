import curry from 'next/dist/compiled/lodash.curry';
export const loader = curry(function loader(rule, config) {
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
export const unshiftLoader = curry(function unshiftLoader(rule, config) {
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
export const plugin = curry(function plugin(p, config) {
    if (!config.plugins) {
        config.plugins = [];
    }
    config.plugins.push(p);
    return config;
});

//# sourceMappingURL=helpers.js.map