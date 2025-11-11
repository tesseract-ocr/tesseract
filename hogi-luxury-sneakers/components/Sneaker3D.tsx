'use client';

import { useRef } from 'react';
import { Canvas, useFrame } from '@react-three/fiber';
import { OrbitControls, PerspectiveCamera, Environment } from '@react-three/drei';
import * as THREE from 'three';

function SneakerModel({ color = '#000000' }: { color?: string }) {
  const meshRef = useRef<THREE.Group>(null);

  useFrame((state) => {
    if (meshRef.current) {
      meshRef.current.rotation.y = state.clock.getElapsedTime() * 0.3;
    }
  });

  return (
    <group ref={meshRef} position={[0, -0.5, 0]}>
      {/* Sole */}
      <mesh position={[0, 0, 0]} castShadow>
        <boxGeometry args={[2.5, 0.3, 1]} />
        <meshStandardMaterial color="#ffffff" roughness={0.3} metalness={0.8} />
      </mesh>

      {/* Main body */}
      <mesh position={[0, 0.5, 0]} castShadow>
        <boxGeometry args={[2.3, 0.8, 0.9]} />
        <meshStandardMaterial color={color} roughness={0.2} metalness={0.1} />
      </mesh>

      {/* Toe cap */}
      <mesh position={[1.2, 0.3, 0]} castShadow>
        <sphereGeometry args={[0.5, 32, 32]} />
        <meshStandardMaterial color={color} roughness={0.2} metalness={0.1} />
      </mesh>

      {/* Heel */}
      <mesh position={[-1, 0.6, 0]} castShadow>
        <boxGeometry args={[0.6, 1, 0.9]} />
        <meshStandardMaterial color={color} roughness={0.2} metalness={0.1} />
      </mesh>

      {/* Tongue */}
      <mesh position={[0.2, 1, 0]} castShadow>
        <boxGeometry args={[0.6, 0.8, 0.1]} />
        <meshStandardMaterial color={color} roughness={0.3} metalness={0.05} />
      </mesh>

      {/* Laces */}
      {[0.8, 0.4, 0, -0.4].map((x, i) => (
        <mesh key={i} position={[x, 0.9, 0]} castShadow>
          <cylinderGeometry args={[0.03, 0.03, 0.3, 8]} />
          <meshStandardMaterial color="#d4af37" roughness={0.1} metalness={0.9} />
        </mesh>
      ))}

      {/* Logo accent */}
      <mesh position={[-0.8, 0.7, 0.46]} castShadow>
        <circleGeometry args={[0.15, 32]} />
        <meshStandardMaterial color="#d4af37" roughness={0.1} metalness={0.9} />
      </mesh>
    </group>
  );
}

export default function Sneaker3D({ color = '#000000', autoRotate = true }: { color?: string; autoRotate?: boolean }) {
  return (
    <div className="w-full h-full">
      <Canvas shadows>
        <PerspectiveCamera makeDefault position={[3, 2, 5]} fov={50} />
        <ambientLight intensity={0.5} />
        <spotLight position={[10, 10, 10]} angle={0.15} penumbra={1} intensity={1} castShadow />
        <pointLight position={[-10, -10, -10]} intensity={0.5} />
        <SneakerModel color={color} />
        <OrbitControls 
          enableZoom={false} 
          autoRotate={autoRotate} 
          autoRotateSpeed={2}
          minPolarAngle={Math.PI / 3}
          maxPolarAngle={Math.PI / 2}
        />
        <Environment preset="city" />
      </Canvas>
    </div>
  );
}
