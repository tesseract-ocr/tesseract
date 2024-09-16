import * as React from 'react';
export type ToastProps = {
    children?: React.ReactNode;
    onClick?: () => void;
    className?: string;
};
export declare const Toast: React.FC<ToastProps>;
