'use client';

import React, { useState, useRef } from 'react';
import axios from 'axios';
import { saveAs } from 'file-saver';
import Webcam from 'react-webcam';

const ImageUploader = () => {
  const [selectedFile, setSelectedFile] = useState(null);
  const [result, setResult] = useState('');
  const [isLoading, setIsLoading] = useState(false);
  const [showCamera, setShowCamera] = useState(false);
  const webcamRef = useRef(null);

  const handleFileChange = (event) => {
    if (event.target.files && event.target.files.length > 0) {
      setSelectedFile(event.target.files[0]);
      handleFileUpload(event.target.files[0]);
    }
  };

  const handleCameraCapture = async () => {
    const imageSrc = webcamRef.current.getScreenshot();
    if (imageSrc) {
      // Convert base64 to blob
      const response = await fetch(imageSrc);
      const blob = await response.blob();
      const file = new File([blob], "webcam-capture.jpg", { type: 'image/jpeg' });
      setSelectedFile(file);
      handleFileUpload(file);
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

  const handleDownload = () => {
    if (result) {
      const blob = new Blob([result], { type: 'application/msword' });
      saveAs(blob, 'ocr_result.doc');
    }
  };

  const toggleCamera = () => {
    setShowCamera(!showCamera);
  };

  return (
    <div className="max-w-md mx-auto mt-8">
      <div className="space-y-4">
        <div className="flex space-x-2">
          <input
            type="file"
            accept="image/*"
            onChange={handleFileChange}
            className="block w-full text-sm text-gray-500
              file:mr-4 file:py-2 file:px-4
              file:rounded-full file:border-0
              file:text-sm file:font-semibold
              file:bg-violet-50 file:text-violet-700
              hover:file:bg-violet-100"
          />
          <button
            onClick={toggleCamera}
            className="px-4 py-2 bg-blue-500 text-white rounded hover:bg-blue-600 transition-colors"
          >
            {showCamera ? 'Hide Camera' : 'Use Camera'}
          </button>
        </div>

        {showCamera && (
          <div className="relative">
            <Webcam
              ref={webcamRef}
              screenshotFormat="image/jpeg"
              className="w-full rounded"
            />
            <button
              onClick={handleCameraCapture}
              className="mt-2 px-4 py-2 bg-green-500 text-white rounded hover:bg-green-600 transition-colors w-full"
            >
              Capture Photo
            </button>
          </div>
        )}

        {isLoading && <p className="mt-4">Processing...</p>}
        
        {result && (
          <div className="mt-4">
            <h3 className="font-bold">OCR Result:</h3>
            <pre className="mt-2 p-2 bg-gray-100 rounded">{result}</pre>
            <button
              onClick={handleDownload}
              className="mt-4 px-4 py-2 bg-green-500 text-white rounded hover:bg-green-600 transition-colors"
            >
              Download as .doc
            </button>
          </div>
        )}
      </div>
    </div>
  );
};

export default ImageUploader;