import * as React from 'react';
export type LeftRightDialogHeaderProps = {
    children?: React.ReactNode;
    className?: string;
    previous: (() => void) | null;
    next: (() => void) | null;
    close?: () => void;
};
declare const LeftRightDialogHeader: React.FC<LeftRightDialogHeaderProps>;
export { LeftRightDialogHeader };
