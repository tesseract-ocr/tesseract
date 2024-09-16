export default function uploadTrace({ traceUploadUrl, mode, projectDir, distDir, sync, }: {
    traceUploadUrl: string;
    mode: 'dev';
    projectDir: string;
    distDir: string;
    sync?: boolean;
}): void;
