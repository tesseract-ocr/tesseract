"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getESLintPromptValues: null,
    getESLintStrictValue: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getESLintPromptValues: function() {
        return getESLintPromptValues;
    },
    getESLintStrictValue: function() {
        return getESLintStrictValue;
    }
});
const _findup = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/find-up"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const getESLintStrictValue = async (cwd)=>{
    const tsConfigLocation = await (0, _findup.default)('tsconfig.json', {
        cwd
    });
    const hasTSConfig = tsConfigLocation !== undefined;
    return {
        title: 'Strict',
        recommended: true,
        config: {
            extends: hasTSConfig ? [
                'next/core-web-vitals',
                'next/typescript'
            ] : 'next/core-web-vitals'
        }
    };
};
const getESLintPromptValues = async (cwd)=>{
    return [
        await getESLintStrictValue(cwd),
        {
            title: 'Base',
            config: {
                extends: 'next'
            }
        },
        {
            title: 'Cancel',
            config: null
        }
    ];
};

//# sourceMappingURL=getESLintPromptValues.js.map