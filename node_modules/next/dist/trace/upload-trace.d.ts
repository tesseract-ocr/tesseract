export default function uploadTrace({ traceUploadUrl, mode, projectDir, distDir, isTurboSession, sync, }: {
    traceUploadUrl: string;
    mode: 'dev' | 'build';
    projectDir: string;
    distDir: string;
    isTurboSession: boolean;
    sync?: boolean;
}): void;
