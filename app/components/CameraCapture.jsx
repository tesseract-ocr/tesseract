import React, { useRef, useState } from 'react';
import Webcam from 'react-webcam';
import '../style/CameraCapture.css';

const CameraCapture = ({ onCapture }) => {
  const webcamRef = useRef(null);
  const [capturedImages, setCapturedImages] = useState([]);

  const handleCameraCapture = async () => {
    const imageSrc = webcamRef.current.getScreenshot();
    if (imageSrc) {
      const response = await fetch(imageSrc);
      const blob = await response.blob();
      const file = new File([blob], `webcam-capture-${Date.now()}.jpg`, { type: 'image/jpeg' });
      setCapturedImages(prev => [...prev, file]);
      onCapture(file);
    }
  };

  return (
    <div className="camera-container">
      <Webcam
        ref={webcamRef}
        screenshotFormat="image/jpeg"
        className="webcam"
      />
      <button
        onClick={handleCameraCapture}
        className="capture-button"
      >
        Capture Photo ({capturedImages.length} captured)
      </button>
    </div>
  );
};

export default CameraCapture; 