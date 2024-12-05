'use client';

import dynamic from 'next/dynamic';

const ImageUploader = dynamic(() => import('./components/ImageUploader'), { ssr: false });

export default function UploadPage() {
  return (
    <div className="container mx-auto p-4">
      <h1 style={{textAlign: 'center'}}>Image Upload and OCR</h1>
      <ImageUploader />
    </div>
  );
}