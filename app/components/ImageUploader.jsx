'use client'; // Add this line at the top of the file

import React, { useState } from 'react';

const ImageUploader = () => {
  const [isProcessing, setIsProcessing] = useState(false);
  const [ocrResult, setOcrResult] = useState('');

  const performOCR = async (file) => {
    setIsProcessing(true);
    setOcrResult('');

    const formData = new FormData();
    formData.append('image', file);

    try {
      const response = await fetch('/api/ocr', {
        method: 'POST',
        body: formData,
      });

      if (!response.ok) {
        throw new Error('OCR request failed');
      }

      const data = await response.json();
      setOcrResult(data.text);
    } catch (error) {
      console.error('Error performing OCR:', error);
      setOcrResult('Error processing image');
    } finally {
      setIsProcessing(false);
    }
  };

  const handleFileChange = (event) => {
    if (event.target.files && event.target.files.length > 0) {
      const file = event.target.files[0];
      performOCR(file);
    }
  };

  const downloadAsDoc = () => {
    // Replace newlines with <br> tags for HTML
    const formattedText = ocrResult.replace(/\n/g, '<br>');
    
    // Create a blob with HTML content
    const blob = new Blob([`<html><body>${formattedText}</body></html>`], 
      { type: 'application/msword;charset=utf-8' });
    
    const url = window.URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = 'ocr_result.doc';
    
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    
    window.URL.revokeObjectURL(url);
  };

  return (
    <div className="max-w-md mx-auto mt-8">
      <p className="mb-4">Choose an image file or take a photo to upload and perform OCR.</p>
      <input
        type="file"
        accept="image/*"
        onChange={handleFileChange}
        className="block w-full text-sm text-gray-500 mb-4"
      />
      {isProcessing && <p>Processing image...</p>}
      {ocrResult && (
        <div className="mt-4">
          <h3 className="font-bold">OCR Result:</h3>
          <pre className="mt-2 p-2 bg-gray-100 rounded">{ocrResult}</pre>
          <button
            onClick={downloadAsDoc}
            className="mt-4 px-4 py-2 bg-green-500 text-white rounded hover:bg-green-600 transition-colors"
          >
            Download as .doc
          </button>
        </div>
      )}
    </div>
  );
};

export default ImageUploader;