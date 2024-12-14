import findUp from 'next/dist/compiled/find-up';
export const getESLintStrictValue = async (cwd)=>{
    const tsConfigLocation = await findUp('tsconfig.json', {
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
export const getESLintPromptValues = async (cwd)=>{
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