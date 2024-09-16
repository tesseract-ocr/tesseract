import React from 'react';
import dynamic from 'next/dynamic';

const ImageUploader = dynamic(() => import('../components/ImageUploader'), { ssr: false });

export default function UploadPage() {
  return (
    <div>
      <h1>Image Upload and OCR</h1>
      <p>Choose an image file or take a photo to upload and perform OCR.</p>
      <ImageUploader />
    </div>
  );
}