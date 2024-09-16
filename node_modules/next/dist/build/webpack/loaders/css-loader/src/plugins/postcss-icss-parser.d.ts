declare const plugin: {
    (options?: any): {
        postcssPlugin: string;
        OnceExit(root: any): Promise<void>;
    };
    postcss: boolean;
};
export default plugin;
