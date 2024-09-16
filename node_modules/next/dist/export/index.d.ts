import type { ExportAppResult, ExportAppOptions } from './types';
import '../server/require-hook';
import type { Span } from '../trace';
export declare class ExportError extends Error {
    code: string;
}
export declare function exportAppImpl(dir: string, options: Readonly<ExportAppOptions>, span: Span): Promise<ExportAppResult | null>;
export default function exportApp(dir: string, options: ExportAppOptions, span: Span): Promise<ExportAppResult | null>;
