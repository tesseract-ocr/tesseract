/// <reference types="react" />
type PortalProps = {
    children: React.ReactNode;
    type: string;
};
export declare const Portal: ({ children, type }: PortalProps) => import("react").ReactPortal | null;
export {};
