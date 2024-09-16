/**
 * For server-side CSS imports, we need to ignore the actual module content but
 * still trigger the hot-reloading diff mechanism. So here we put the content
 * inside a comment.
 */
import type webpack from 'webpack';
type NextServerCSSLoaderOptions = {
    cssModules: boolean;
};
declare const NextServerCSSLoader: webpack.LoaderDefinitionFunction<NextServerCSSLoaderOptions>;
export default NextServerCSSLoader;
