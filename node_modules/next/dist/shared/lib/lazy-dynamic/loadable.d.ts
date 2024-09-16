/// <reference types="react" />
import type { ComponentModule } from './types';
interface LoadableOptions {
    loader?: () => Promise<React.ComponentType<any> | ComponentModule<any>>;
    loading?: React.ComponentType<any> | null;
    ssr?: boolean;
    modules?: string[];
}
declare function Loadable(options: LoadableOptions): {
    (props: any): import("react/jsx-runtime").JSX.Element;
    displayName: string;
};
export default Loadable;
