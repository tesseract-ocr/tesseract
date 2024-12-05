import React, { useRef, useState } from 'react';
import Webcam from 'react-webcam';

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
        Capture Photo ({capturedImages.length} captured)
      </button>
    </div>
  );
};

export default CameraCapture; 