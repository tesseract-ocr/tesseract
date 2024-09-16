declare const plugin: {
    (options?: any): {
        postcssPlugin: string;
        prepare(result: any): {
            AtRule: {
                import(atRule: any): void;
            };
            OnceExit(): Promise<void>;
        };
    };
    postcss: boolean;
};
export default plugin;
