'use client';

import React, { useState } from 'react';
import axios from 'axios';
import { saveAs } from 'file-saver';
import CameraCapture from './CameraCapture';
import '../style/ImageUploader.css';

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
    <div className="container">
      <div className="wrapper">
        <h2 className="title">Image Text Extractor</h2>
        <div className="content">
          <div className="upload-section">
            <div className="file-input-wrapper">
              <input
                type="file"
                accept="image/*"
                onChange={handleFileChange}
                multiple
                className="file-input"
              />
            </div>
            <button
              onClick={toggleCamera}
              className="camera-button"
            >
              {showCamera ? 'Hide Camera' : 'Use Camera'}
            </button>
          </div>

          {showCamera && (
            <div className="camera-container">
              <CameraCapture onCapture={(file) => processFiles([file])} />
            </div>
          )}

          {isLoading && (
            <div className="loading">
              <div className="spinner"></div>
              <p className="loading-text">Processing images...</p>
            </div>
          )}
          
          {results.length > 0 && (
            <div className="results-section">
              <div className="results-header">
                <h3 className="results-title">OCR Results</h3>
                <button onClick={handleDownload} className="download-button">
                  <svg className="download-icon" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} 
                      d="M4 16v1a3 3 0 003 3h10a3 3 0 003-3v-1m-4-4l-4 4m0 0l-4-4m4 4V4" />
                  </svg>
                  Download Results
                </button>
              </div>
              
              <div className="results-list">
                {results.map((result, index) => (
                  <div key={index} className="result-item">
                    <h4 className="result-filename">{result.file}</h4>
                    <pre className="result-text">{result.text}</pre>
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