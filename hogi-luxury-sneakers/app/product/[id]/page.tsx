'use client';

import { useState } from 'react';
import { useParams, useRouter } from 'next/navigation';
import { products } from '@/data/products';
import dynamic from 'next/dynamic';
import Link from 'next/link';

const Sneaker3D = dynamic(() => import('@/components/Sneaker3D'), {
  ssr: false,
});

export default function ProductPage() {
  const params = useParams();
  const router = useRouter();
  const product = products.find(p => p.id === params.id);

  const [selectedSize, setSelectedSize] = useState<number | null>(null);
  const [selectedColor, setSelectedColor] = useState<string>(product?.colors[0] || '');
  const [quantity, setQuantity] = useState(1);

  if (!product) {
    return (
      <div className="pt-20 min-h-screen bg-black flex items-center justify-center">
        <div className="text-center">
          <h1 className="text-4xl font-bold text-white mb-4">Product Not Found</h1>
          <Link href="/shop" className="text-gold hover:underline">
            Return to Shop
          </Link>
        </div>
      </div>
    );
  }

  const getColorHex = (colorName: string) => {
    const colorMap: { [key: string]: string } = {
      'Black': '#000000',
      'White': '#ffffff',
      'Gold': '#d4af37',
      'Platinum': '#e5e4e2',
      'Navy': '#001f3f',
      'Burgundy': '#800020',
      'Forest Green': '#228B22',
      'Rose Gold': '#b76e79',
      'Cognac': '#9A463D',
      'Olive': '#808000',
      'Red': '#ff0000',
      'Blue': '#0000ff',
      'Silver': '#c0c0c0',
      'Chrome': '#e8e8e8',
      'Titanium': '#878681',
      'Grey': '#808080',
      'Tan': '#d2b48c',
    };
    return colorMap[colorName] || '#d4af37';
  };

  const handleAddToCart = () => {
    if (!selectedSize) {
      alert('Please select a size');
      return;
    }

    const cartItem = {
      ...product,
      quantity,
      selectedSize,
      selectedColor,
    };

    const existingCart = JSON.parse(localStorage.getItem('cart') || '[]');
    const existingItemIndex = existingCart.findIndex(
      (item: any) => item.id === product.id && item.selectedSize === selectedSize && item.selectedColor === selectedColor
    );

    if (existingItemIndex > -1) {
      existingCart[existingItemIndex].quantity += quantity;
    } else {
      existingCart.push(cartItem);
    }

    localStorage.setItem('cart', JSON.stringify(existingCart));
    router.push('/cart');
  };

  return (
    <div className="pt-20 min-h-screen bg-gradient-to-b from-black via-gray-900 to-black">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-12">
        {/* Breadcrumb */}
        <div className="mb-8 flex items-center gap-2 text-sm text-gray-400">
          <Link href="/" className="hover:text-gold transition-colors">Home</Link>
          <span>/</span>
          <Link href="/shop" className="hover:text-gold transition-colors">Shop</Link>
          <span>/</span>
          <span className="text-white">{product.name}</span>
        </div>

        <div className="grid grid-cols-1 lg:grid-cols-2 gap-12">
          {/* 3D Viewer */}
          <div className="bg-gradient-to-br from-gray-900 to-black rounded-2xl border border-gold/20 p-8">
            <div className="aspect-square">
              <Sneaker3D color={getColorHex(selectedColor)} autoRotate={true} />
            </div>
            {product.new && (
              <div className="mt-4 inline-block bg-gold text-black px-4 py-2 rounded-full text-sm font-bold">
                NEW ARRIVAL
              </div>
            )}
          </div>

          {/* Product Details */}
          <div className="space-y-6">
            <div>
              <div className="text-sm text-gold font-semibold mb-2">{product.category}</div>
              <h1 className="text-4xl md:text-5xl font-bold text-white mb-4">{product.name}</h1>
              <div className="text-4xl font-bold text-gold mb-6">${product.price}</div>
              <p className="text-gray-300 text-lg leading-relaxed">{product.description}</p>
            </div>

            {/* Color Selection */}
            <div>
              <label className="block text-white font-semibold mb-3">
                Color: <span className="text-gold">{selectedColor}</span>
              </label>
              <div className="flex flex-wrap gap-3">
                {product.colors.map((color) => (
                  <button
                    key={color}
                    onClick={() => setSelectedColor(color)}
                    className={`w-12 h-12 rounded-full border-4 transition-all duration-300 ${
                      selectedColor === color
                        ? 'border-gold scale-110'
                        : 'border-gray-700 hover:border-gray-500'
                    }`}
                    style={{ backgroundColor: getColorHex(color) }}
                    title={color}
                  />
                ))}
              </div>
            </div>

            {/* Size Selection */}
            <div>
              <label className="block text-white font-semibold mb-3">
                Size (US): {selectedSize && <span className="text-gold">{selectedSize}</span>}
              </label>
              <div className="grid grid-cols-6 gap-2">
                {product.sizes.map((size) => (
                  <button
                    key={size}
                    onClick={() => setSelectedSize(size)}
                    className={`py-3 rounded-lg font-semibold transition-all duration-300 ${
                      selectedSize === size
                        ? 'bg-gradient-to-r from-gold to-yellow-600 text-black'
                        : 'bg-gray-800 text-white hover:bg-gray-700'
                    }`}
                  >
                    {size}
                  </button>
                ))}
              </div>
            </div>

            {/* Quantity */}
            <div>
              <label className="block text-white font-semibold mb-3">Quantity</label>
              <div className="flex items-center gap-4">
                <button
                  onClick={() => setQuantity(Math.max(1, quantity - 1))}
                  className="w-12 h-12 bg-gray-800 text-white rounded-lg hover:bg-gray-700 transition-colors"
                >
                  -
                </button>
                <span className="text-2xl font-bold text-white w-12 text-center">{quantity}</span>
                <button
                  onClick={() => setQuantity(quantity + 1)}
                  className="w-12 h-12 bg-gray-800 text-white rounded-lg hover:bg-gray-700 transition-colors"
                >
                  +
                </button>
              </div>
            </div>

            {/* Add to Cart */}
            <button
              onClick={handleAddToCart}
              className="w-full py-4 bg-gradient-to-r from-gold to-yellow-600 text-black font-bold text-lg rounded-full hover:shadow-2xl hover:shadow-gold/50 transition-all duration-300"
            >
              Add to Cart - ${(product.price * quantity).toLocaleString()}
            </button>

            {/* Features */}
            <div className="border-t border-gold/20 pt-6 space-y-4">
              <div className="flex items-start gap-3">
                <svg className="w-6 h-6 text-gold flex-shrink-0 mt-1" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 13l4 4L19 7" />
                </svg>
                <div>
                  <div className="text-white font-semibold">Premium Materials</div>
                  <div className="text-gray-400 text-sm">Handcrafted with the finest materials</div>
                </div>
              </div>
              <div className="flex items-start gap-3">
                <svg className="w-6 h-6 text-gold flex-shrink-0 mt-1" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 13l4 4L19 7" />
                </svg>
                <div>
                  <div className="text-white font-semibold">Limited Edition</div>
                  <div className="text-gray-400 text-sm">Exclusive release with certificate of authenticity</div>
                </div>
              </div>
              <div className="flex items-start gap-3">
                <svg className="w-6 h-6 text-gold flex-shrink-0 mt-1" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 13l4 4L19 7" />
                </svg>
                <div>
                  <div className="text-white font-semibold">Free Shipping</div>
                  <div className="text-gray-400 text-sm">Complimentary worldwide shipping</div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
