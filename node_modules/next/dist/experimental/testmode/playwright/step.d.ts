import type { TestInfo } from '@playwright/test';
export interface StepProps {
    category: string;
    title: string;
    apiName?: string;
    params?: Record<string, string | number | boolean | null | undefined>;
}
type Complete = (result: {
    error?: any;
}) => void;
export declare function step<T>(testInfo: TestInfo, props: StepProps, handler: (complete: Complete) => Promise<Awaited<T>>): Promise<Awaited<T>>;
export {};
