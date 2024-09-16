import type { ReactElement } from 'react';
interface BailoutToCSRProps {
    reason: string;
    children: ReactElement;
}
/**
 * If rendered on the server, this component throws an error
 * to signal Next.js that it should bail out to client-side rendering instead.
 */
export declare function BailoutToCSR({ reason, children }: BailoutToCSRProps): ReactElement<any, string | import("react").JSXElementConstructor<any>>;
export {};
