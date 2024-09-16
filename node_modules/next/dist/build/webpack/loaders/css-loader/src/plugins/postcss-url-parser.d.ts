declare const plugin: {
    (options?: any): {
        postcssPlugin: string;
        prepare(result: any): {
            Declaration(declaration: any): void;
            OnceExit(): Promise<void>;
        };
    };
    postcss: boolean;
};
export default plugin;
