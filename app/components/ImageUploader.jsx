'use client';

import React, { useState } from 'react';
import axios from 'axios';
import { saveAs } from 'file-saver';
import CameraCapture from './CameraCapture';

const ImageUploader = () => {
  const [selectedFiles, setSelectedFiles] = useState([]);
  const [results, setResults] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [showCamera, setShowCamera] = useState(false);
  const [currentProcessingIndex, setCurrentProcessingIndex] = useState(0);

  const handleFileChange = (event) => {
    if (event.target.files && event.target.files.length > 0) {
      const newFiles = Array.from(event.target.files);
      setSelectedFiles(prev => [...prev, ...newFiles]);
      processFiles(newFiles);
    }
  };

  const processFiles = async (files) => {
    setIsLoading(true);
    
    for (const file of files) {
      await handleFileUpload(file);
    }
    
    setIsLoading(false);
  };

  const handleFileUpload = async (file) => {
    const formData = new FormData();
    formData.append('image', file);

    try {
      const response = await axios.post('/api/ocr', formData, {
        headers: { 'Content-Type': 'multipart/form-data' },
      });
      setResults(prev => [...prev, { file: file.name, text: response.data.text }]);
    } catch (error) {
      console.error('Error uploading image:', error);
      setResults(prev => [...prev, { file: file.name, text: 'Error processing image' }]);
    }
  };

  const handleDownload = () => {
    if (results.length > 0) {
      const combinedText = results.map(r => `File: ${r.file}\n${r.text}\n\n`).join('---\n');
      const blob = new Blob([combinedText], { type: 'application/msword' });
      saveAs(blob, 'ocr_results.doc');
    }
  };

  const toggleCamera = () => {
    setShowCamera(!showCamera);
  };

  return (
    <div className="min-h-screen bg-gray-50 py-12 px-4 sm:px-6 lg:px-8">
      <div className="max-w-2xl mx-auto bg-white rounded-lg shadow-lg p-8">
        <h2 className="text-2xl font-bold text-gray-900 mb-8 text-center">
          Image Text Extractor
        </h2>
        
        <div className="space-y-6">
          <div className="flex flex-col sm:flex-row sm:space-x-4 space-y-4 sm:space-y-0">
            <div className="flex-1">
              <input
                type="file"
                accept="image/*"
                onChange={handleFileChange}
                multiple
                className="block w-full text-sm text-gray-500
                  file:mr-4 file:py-3 file:px-4
                  file:rounded-lg file:border-0
                  file:text-sm file:font-semibold
                  file:bg-violet-100 file:text-violet-700
                  hover:file:bg-violet-200 
                  cursor-pointer
                  border border-gray-300 rounded-lg"
              />
            </div>
            <button
              onClick={toggleCamera}
              className="px-6 py-3 bg-blue-600 text-white rounded-lg hover:bg-blue-700 
                transition-colors duration-200 font-medium shadow-sm"
            >
              {showCamera ? 'Hide Camera' : 'Use Camera'}
            </button>
          </div>

          {showCamera && (
            <div className="border-2 border-dashed border-gray-300 rounded-lg p-4">
              <CameraCapture onCapture={(file) => processFiles([file])} />
            </div>
          )}

          {isLoading && (
            <div className="text-center py-4">
              <div className="animate-spin rounded-full h-10 w-10 border-b-2 border-blue-600 mx-auto"></div>
              <p className="mt-2 text-gray-600">Processing images...</p>
            </div>
          )}
          
          {results.length > 0 && (
            <div className="mt-8 space-y-6">
              <div className="flex items-center justify-between">
                <h3 className="text-xl font-bold text-gray-900">OCR Results</h3>
                <button
                  onClick={handleDownload}
                  className="inline-flex items-center px-4 py-2 bg-green-600 text-white rounded-lg 
                    hover:bg-green-700 transition-colors duration-200 font-medium shadow-sm"
                >
                  <svg className="w-5 h-5 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} 
                      d="M4 16v1a3 3 0 003 3h10a3 3 0 003-3v-1m-4-4l-4 4m0 0l-4-4m4 4V4" />
                  </svg>
                  Download Results
                </button>
              </div>
              
              <div className="space-y-4">
                {results.map((result, index) => (
                  <div key={index} className="bg-gray-50 rounded-lg p-4 shadow-sm">
                    <h4 className="font-medium text-gray-900 mb-2">{result.file}</h4>
                    <pre className="bg-white p-4 rounded-lg border border-gray-200 text-sm text-gray-700 
                      overflow-x-auto">{result.text}</pre>
                  </div>
                ))}
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  );
};

export default ImageUploader;