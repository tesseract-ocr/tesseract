'use client'; // Add this line at the top of the file

import React, { useState } from 'react';
import axios from 'axios';

const ImageUploader: React.FC = () => {
  const [selectedFile, setSelectedFile] = useState<File | null>(null);
  const [result, setResult] = useState<string>('');
  const [isLoading, setIsLoading] = useState<boolean>(false);

  const handleFileChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    if (event.target.files && event.target.files.length > 0) {
      setSelectedFile(event.target.files[0]);
      handleFileUpload(event.target.files[0]);
    }
  };

  const handleCapturePhoto = (event: React.ChangeEvent<HTMLInputElement>) => {
    if (event.target.files && event.target.files.length > 0) {
      setSelectedFile(event.target.files[0]);
      handleFileUpload(event.target.files[0]);
    }
  };

  const handleFileUpload = async (file: File) => {
    setIsLoading(true);
    const formData = new FormData();
    formData.append('image', file);

    try {
      const response = await axios.post('/api/ocr', formData, {
        headers: { 'Content-Type': 'multipart/form-data' },
      });
      setResult(response.data.text);
    } catch (error) {
      console.error('Error uploading image:', error);
      alert('An error occurred while processing the image');
    } finally {
      setIsLoading(false);
    }
  };

  return (
    <div>
      <input
        type="file"
        accept="image/*"
        onChange={handleFileChange}
        style={{ display: 'none' }}
        id="fileInput"
      />
      <label htmlFor="fileInput" className="btn btn-primary mr-2">
        Choose File
      </label>
      
      <input
        type="file"
        accept="image/*"
        capture="environment"
        onChange={handleCapturePhoto}
        style={{ display: 'none' }}
        id="cameraInput"
      />
      <label htmlFor="cameraInput" className="btn btn-secondary">
        Take Photo
      </label>
      <input
        type="file"
        accept="image/*"
        capture="environment"
        onChange={handleCapturePhoto}
        style={{ display: 'none' }}
        id="cameraInput"
      />
      {isLoading && <p>Processing...</p>}
      {result && (
        <div>
          <h3>OCR Result:</h3>
          <pre>{result}</pre>
        </div>
      )}
    </div>
  );
};

export default ImageUploader;