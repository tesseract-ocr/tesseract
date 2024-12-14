import React, { useRef, useState } from 'react';
import Webcam from 'react-webcam';
import '../style/CameraCapture.css';

const CameraCapture = ({ onCapture }) => {
  const webcamRef = useRef(null);
  const [capturedImages, setCapturedImages] = useState([]);
  const [showCaptureMore, setShowCaptureMore] = useState(false);
  const [readyToProcess, setReadyToProcess] = useState(false);

  const handleCameraCapture = async () => {
    const imageSrc = webcamRef.current.getScreenshot();
    if (imageSrc) {
      const response = await fetch(imageSrc);
      const blob = await response.blob();
      const file = new File([blob], `webcam-capture-${Date.now()}.jpg`, { type: 'image/jpeg' });
      setCapturedImages(prev => [...prev, file]);
      setShowCaptureMore(true);
    }
  };

  const handleProcessImages = () => {
    capturedImages.forEach(file => onCapture(file));
    setCapturedImages([]); // Clear captured images after processing
    setReadyToProcess(false);
    setShowCaptureMore(false);
  };

  return (
    <div className="camera-container">
      <Webcam
        ref={webcamRef}
        screenshotFormat="image/jpeg"
        className="webcam"
      />
      
      {!showCaptureMore && (
        <button
          onClick={handleCameraCapture}
          className="capture-button"
        >
          Capture Photo ({capturedImages.length} captured)
        </button>
      )}

      {capturedImages.length > 0 && (
        <div className="captured-images">
          <h4>Captured Photos ({capturedImages.length}):</h4>
          <ul>
            {capturedImages.map((file, index) => (
              <li key={index}>Photo {index + 1}</li>
            ))}
          </ul>
        </div>
      )}

      {showCaptureMore && (
        <div className="capture-more-prompt">
          <p>Would you like to capture more photos?</p>
          <div className="capture-more-buttons">
            <button
              onClick={() => {
                setShowCaptureMore(false);
                handleCameraCapture();
              }}
              className="capture-more-button"
            >
              Yes, Capture More
            </button>
            <button
              onClick={() => {
                setShowCaptureMore(false);
                setReadyToProcess(true);
              }}
              className="finish-capture-button"
            >
              No, I'm Done Capturing
            </button>
          </div>
        </div>
      )}

      {readyToProcess && capturedImages.length > 0 && (
        <div className="process-section">
          <button 
            onClick={handleProcessImages}
            className="process-button"
          >
            Process {capturedImages.length} Photos
          </button>
        </div>
      )}
    </div>
  );
};

export default CameraCapture; 