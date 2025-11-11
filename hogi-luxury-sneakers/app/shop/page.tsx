'use client';

import { useState } from 'react';
import Link from 'next/link';
import { products } from '@/data/products';
import dynamic from 'next/dynamic';

const Sneaker3D = dynamic(() => import('@/components/Sneaker3D'), {
  ssr: false,
});

export default function ShopPage() {
  const [selectedCategory, setSelectedCategory] = useState<string>('All');
  const [priceRange, setPriceRange] = useState<string>('All');

  const categories = ['All', ...Array.from(new Set(products.map(p => p.category)))];

  const filteredProducts = products.filter(product => {
    const categoryMatch = selectedCategory === 'All' || product.category === selectedCategory;
    
    let priceMatch = true;
    if (priceRange === '500-600') {
      priceMatch = product.price >= 500 && product.price < 600;
    } else if (priceRange === '600-700') {
      priceMatch = product.price >= 600 && product.price < 700;
    } else if (priceRange === '700+') {
      priceMatch = product.price >= 700;
    }

    return categoryMatch && priceMatch;
  });

  return (
    <div className="pt-20 min-h-screen bg-gradient-to-b from-black via-gray-900 to-black">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-12">
        {/* Header */}
        <div className="text-center mb-12">
          <h1 className="text-5xl md:text-6xl font-bold text-white mb-4">
            Luxury Collection
          </h1>
          <p className="text-xl text-gray-400">
            Discover our exclusive range of premium sneakers
          </p>
        </div>

        {/* Filters */}
        <div className="mb-12 flex flex-col md:flex-row gap-6 items-center justify-between bg-gray-900/50 p-6 rounded-2xl border border-gold/20">
          <div className="flex flex-wrap gap-3">
            <span className="text-gold font-semibold">Category:</span>
            {categories.map(category => (
              <button
                key={category}
                onClick={() => setSelectedCategory(category)}
                className={`px-4 py-2 rounded-full font-medium transition-all duration-300 ${
                  selectedCategory === category
                    ? 'bg-gradient-to-r from-gold to-yellow-600 text-black'
                    : 'bg-gray-800 text-gray-300 hover:bg-gray-700'
                }`}
              >
                {category}
              </button>
            ))}
          </div>

          <div className="flex flex-wrap gap-3 items-center">
            <span className="text-gold font-semibold">Price:</span>
            {['All', '500-600', '600-700', '700+'].map(range => (
              <button
                key={range}
                onClick={() => setPriceRange(range)}
                className={`px-4 py-2 rounded-full font-medium transition-all duration-300 ${
                  priceRange === range
                    ? 'bg-gradient-to-r from-gold to-yellow-600 text-black'
                    : 'bg-gray-800 text-gray-300 hover:bg-gray-700'
                }`}
              >
                {range === 'All' ? 'All' : `$${range}`}
              </button>
            ))}
          </div>
        </div>

        {/* Products Grid */}
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-8">
          {filteredProducts.map((product) => (
            <Link
              key={product.id}
              href={`/product/${product.id}`}
              className="group relative bg-gradient-to-br from-gray-900 to-black rounded-2xl overflow-hidden border border-gold/20 hover:border-gold/60 transition-all duration-500 hover:shadow-2xl hover:shadow-gold/20 hover:scale-105"
            >
              <div className="aspect-square bg-gradient-to-br from-gray-800 to-gray-900 flex items-center justify-center relative overflow-hidden">
                <div className="absolute inset-0 bg-gradient-to-t from-black/80 to-transparent z-10"></div>
                <div className="w-full h-full">
                  <Sneaker3D 
                    color={
                      product.colors[0] === 'Black' ? '#000000' : 
                      product.colors[0] === 'White' ? '#ffffff' : 
                      product.colors[0] === 'Gold' ? '#d4af37' :
                      product.colors[0] === 'Platinum' ? '#e5e4e2' :
                      '#d4af37'
                    } 
                    autoRotate={false} 
                  />
                </div>
                {product.new && (
                  <div className="absolute top-4 right-4 z-20 bg-gold text-black px-3 py-1 rounded-full text-xs font-bold">
                    NEW
                  </div>
                )}
              </div>

              <div className="p-5">
                <div className="flex items-start justify-between mb-2">
                  <h3 className="text-lg font-bold text-white group-hover:text-gold transition-colors duration-300 flex-1">
                    {product.name}
                  </h3>
                </div>
                <p className="text-gray-400 text-sm mb-3 line-clamp-2">
                  {product.description}
                </p>
                <div className="flex items-center justify-between">
                  <span className="text-2xl font-bold text-gold">
                    ${product.price}
                  </span>
                  <span className="text-xs text-gray-500 bg-gray-800 px-2 py-1 rounded-full">
                    {product.category}
                  </span>
                </div>
                <div className="mt-3 flex gap-1">
                  {product.colors.slice(0, 3).map((color, idx) => (
                    <div
                      key={idx}
                      className="w-6 h-6 rounded-full border-2 border-gray-700"
                      style={{
                        backgroundColor: 
                          color === 'Black' ? '#000000' :
                          color === 'White' ? '#ffffff' :
                          color === 'Gold' ? '#d4af37' :
                          color === 'Platinum' ? '#e5e4e2' :
                          color === 'Navy' ? '#001f3f' :
                          color === 'Burgundy' ? '#800020' :
                          color === 'Red' ? '#ff0000' :
                          color === 'Blue' ? '#0000ff' :
                          '#808080'
                      }}
                      title={color}
                    />
                  ))}
                  {product.colors.length > 3 && (
                    <div className="w-6 h-6 rounded-full border-2 border-gray-700 bg-gray-800 flex items-center justify-center text-xs text-gray-400">
                      +{product.colors.length - 3}
                    </div>
                  )}
                </div>
              </div>
            </Link>
          ))}
        </div>

        {filteredProducts.length === 0 && (
          <div className="text-center py-20">
            <p className="text-2xl text-gray-400">No products found matching your filters</p>
          </div>
        )}
      </div>
    </div>
  );
}
