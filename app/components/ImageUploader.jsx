'use client';

import React, { useState } from 'react';
import axios from 'axios';

const ImageUploader = () => {
  const [selectedFile, setSelectedFile] = useState(null);
  const [result, setResult] = useState('');
  const [isLoading, setIsLoading] = useState(false);

  const handleFileChange = (event) => {
    if (event.target.files && event.target.files.length > 0) {
      setSelectedFile(event.target.files[0]);
    }
  };

  const handleUpload = async () => {
    if (!selectedFile) {
      alert('Please select an image file or take a photo');
      return;
    }

    await handleFileUpload(selectedFile);
  };

  const handleCapturePhoto = (event) => {
    const file = event.target.files?.[0];
    if (file) {
      setSelectedFile(file);
    }
  };

  const handleFileUpload = async (file) => {
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
      <label htmlFor="cameraInput" className="btn btn-secondary mr-2">
        Take Photo
      </label>
      
      <button onClick={handleUpload} disabled={!selectedFile || isLoading}>
        {isLoading ? 'Processing...' : 'Upload and Process'}
      </button>
      {selectedFile && <p>Selected: {selectedFile.name}</p>}
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