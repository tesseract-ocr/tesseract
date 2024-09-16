import type { ExportPageInput, ExportPageResult } from './types';
import '../server/node-environment';
export default function exportPage(input: ExportPageInput): Promise<ExportPageResult | undefined>;
