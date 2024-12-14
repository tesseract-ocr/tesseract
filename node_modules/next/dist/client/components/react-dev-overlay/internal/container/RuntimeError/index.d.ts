import type { ReadyRuntimeError } from '../../helpers/get-error-by-type';
export type RuntimeErrorProps = {
    error: ReadyRuntimeError;
};
export declare function RuntimeError({ error }: RuntimeErrorProps): import("react/jsx-runtime").JSX.Element;
export declare const styles: string;
