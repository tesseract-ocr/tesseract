'use client';

import Link from 'next/link';
import { products } from '@/data/products';
import dynamic from 'next/dynamic';

const Sneaker3D = dynamic(() => import('@/components/Sneaker3D'), {
  ssr: false,
  loading: () => (
    <div className="w-full h-full flex items-center justify-center">
      <div className="animate-spin rounded-full h-32 w-32 border-t-2 border-b-2 border-gold"></div>
    </div>
  ),
});

export default function Home() {
  const featuredProducts = products.filter(p => p.featured);

  return (
    <div className="pt-20">
      {/* Hero Section with 3D Sneaker */}
      <section className="relative h-screen flex items-center justify-center overflow-hidden">
        <div className="absolute inset-0 bg-gradient-to-b from-black via-gray-900 to-black"></div>
        
        <div className="relative z-10 max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 grid grid-cols-1 lg:grid-cols-2 gap-12 items-center">
          <div className="text-center lg:text-left space-y-6">
            <h1 className="text-5xl md:text-7xl font-bold">
              <span className="block text-white">Luxury</span>
              <span className="block bg-gradient-to-r from-gold via-yellow-300 to-gold bg-clip-text text-transparent">
                Redefined
              </span>
            </h1>
            <p className="text-xl text-gray-300 max-w-lg">
              Experience the pinnacle of footwear craftsmanship. Each Hogi sneaker is a masterpiece, 
              meticulously designed for those who demand excellence.
            </p>
            <div className="flex flex-col sm:flex-row gap-4 justify-center lg:justify-start">
              <Link 
                href="/shop"
                className="px-8 py-4 bg-gradient-to-r from-gold to-yellow-600 text-black font-bold rounded-full hover:shadow-2xl hover:shadow-gold/50 transition-all duration-300 text-center"
              >
                Explore Collection
              </Link>
              <Link 
                href="/about"
                className="px-8 py-4 border-2 border-gold text-gold font-bold rounded-full hover:bg-gold hover:text-black transition-all duration-300 text-center"
              >
                Our Story
              </Link>
            </div>
          </div>

          <div className="h-[500px] w-full">
            <Sneaker3D color="#d4af37" autoRotate={true} />
          </div>
        </div>

        <div className="absolute bottom-10 left-1/2 transform -translate-x-1/2 animate-bounce">
          <svg className="w-6 h-6 text-gold" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M19 14l-7 7m0 0l-7-7m7 7V3" />
          </svg>
        </div>
      </section>

      {/* Featured Products */}
      <section className="py-20 bg-gradient-to-b from-black to-gray-900">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="text-center mb-16">
            <h2 className="text-4xl md:text-5xl font-bold text-white mb-4">
              Featured Collection
            </h2>
            <p className="text-xl text-gray-400">
              Handpicked masterpieces from our exclusive range
            </p>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-8">
            {featuredProducts.map((product) => (
              <Link 
                key={product.id} 
                href={`/product/${product.id}`}
                className="group relative bg-gradient-to-br from-gray-900 to-black rounded-2xl overflow-hidden border border-gold/20 hover:border-gold/60 transition-all duration-500 hover:shadow-2xl hover:shadow-gold/20"
              >
                <div className="aspect-square bg-gradient-to-br from-gray-800 to-gray-900 flex items-center justify-center relative overflow-hidden">
                  <div className="absolute inset-0 bg-gradient-to-t from-black/80 to-transparent z-10"></div>
                  <div className="w-full h-full">
                    <Sneaker3D color={product.colors[0] === 'Black' ? '#000000' : product.colors[0] === 'White' ? '#ffffff' : '#d4af37'} autoRotate={false} />
                  </div>
                  {product.new && (
                    <div className="absolute top-4 right-4 z-20 bg-gold text-black px-4 py-1 rounded-full text-sm font-bold">
                      NEW
                    </div>
                  )}
                </div>
                
                <div className="p-6">
                  <h3 className="text-xl font-bold text-white mb-2 group-hover:text-gold transition-colors duration-300">
                    {product.name}
                  </h3>
                  <p className="text-gray-400 text-sm mb-4 line-clamp-2">
                    {product.description}
                  </p>
                  <div className="flex items-center justify-between">
                    <span className="text-2xl font-bold text-gold">
                      ${product.price}
                    </span>
                    <span className="text-sm text-gray-500">
                      {product.category}
                    </span>
                  </div>
                </div>
              </Link>
            ))}
          </div>

          <div className="text-center mt-12">
            <Link 
              href="/shop"
              className="inline-block px-8 py-4 bg-gradient-to-r from-gold to-yellow-600 text-black font-bold rounded-full hover:shadow-2xl hover:shadow-gold/50 transition-all duration-300"
            >
              View All Products
            </Link>
          </div>
        </div>
      </section>

      {/* Why Choose Hogi */}
      <section className="py-20 bg-black">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="text-center mb-16">
            <h2 className="text-4xl md:text-5xl font-bold text-white mb-4">
              The Hogi Difference
            </h2>
            <p className="text-xl text-gray-400">
              Uncompromising quality in every detail
            </p>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-3 gap-8">
            <div className="text-center p-8 bg-gradient-to-br from-gray-900 to-black rounded-2xl border border-gold/20">
              <div className="w-16 h-16 bg-gradient-to-br from-gold to-yellow-600 rounded-full flex items-center justify-center mx-auto mb-6">
                <svg className="w-8 h-8 text-black" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 3v4M3 5h4M6 17v4m-2-2h4m5-16l2.286 6.857L21 12l-5.714 2.143L13 21l-2.286-6.857L5 12l5.714-2.143L13 3z" />
                </svg>
              </div>
              <h3 className="text-xl font-bold text-white mb-3">Premium Materials</h3>
              <p className="text-gray-400">
                Only the finest Italian leather, aerospace-grade materials, and precious metal accents
              </p>
            </div>

            <div className="text-center p-8 bg-gradient-to-br from-gray-900 to-black rounded-2xl border border-gold/20">
              <div className="w-16 h-16 bg-gradient-to-br from-gold to-yellow-600 rounded-full flex items-center justify-center mx-auto mb-6">
                <svg className="w-8 h-8 text-black" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M7 21a4 4 0 01-4-4V5a2 2 0 012-2h4a2 2 0 012 2v12a4 4 0 01-4 4zm0 0h12a2 2 0 002-2v-4a2 2 0 00-2-2h-2.343M11 7.343l1.657-1.657a2 2 0 012.828 0l2.829 2.829a2 2 0 010 2.828l-8.486 8.485M7 17h.01" />
                </svg>
              </div>
              <h3 className="text-xl font-bold text-white mb-3">Handcrafted Excellence</h3>
              <p className="text-gray-400">
                Each pair is meticulously crafted by master artisans with decades of experience
              </p>
            </div>

            <div className="text-center p-8 bg-gradient-to-br from-gray-900 to-black rounded-2xl border border-gold/20">
              <div className="w-16 h-16 bg-gradient-to-br from-gold to-yellow-600 rounded-full flex items-center justify-center mx-auto mb-6">
                <svg className="w-8 h-8 text-black" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 8v13m0-13V6a2 2 0 112 2h-2zm0 0V5.5A2.5 2.5 0 109.5 8H12zm-7 4h14M5 12a2 2 0 110-4h14a2 2 0 110 4M5 12v7a2 2 0 002 2h10a2 2 0 002-2v-7" />
                </svg>
              </div>
              <h3 className="text-xl font-bold text-white mb-3">Limited Editions</h3>
              <p className="text-gray-400">
                Exclusive releases with numbered certificates of authenticity
              </p>
            </div>
          </div>
        </div>
      </section>
    </div>
  );
}
