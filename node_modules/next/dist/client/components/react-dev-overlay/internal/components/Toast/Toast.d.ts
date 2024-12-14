import * as React from 'react';
export type ToastProps = React.HTMLProps<HTMLDivElement> & {
    children?: React.ReactNode;
    onClick?: () => void;
    className?: string;
};
export declare const Toast: React.FC<ToastProps>;
