export default function AboutPage() {
  return (
    <div className="pt-20 min-h-screen bg-gradient-to-b from-black via-gray-900 to-black">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-12">
        {/* Hero Section */}
        <div className="text-center mb-16">
          <h1 className="text-5xl md:text-6xl font-bold text-white mb-6">
            The Hogi Story
          </h1>
          <p className="text-xl text-gray-400 max-w-3xl mx-auto">
            Where luxury meets innovation, and craftsmanship defines excellence
          </p>
        </div>

        {/* Mission */}
        <div className="mb-20">
          <div className="bg-gradient-to-br from-gray-900 to-black rounded-2xl border border-gold/20 p-12">
            <h2 className="text-3xl font-bold text-gold mb-6 text-center">Our Mission</h2>
            <p className="text-gray-300 text-lg leading-relaxed text-center max-w-4xl mx-auto">
              At Hogi, we believe that luxury footwear should be more than just a fashion statement. 
              Each pair of sneakers we create is a testament to our commitment to excellence, combining 
              traditional craftsmanship with cutting-edge design. We source only the finest materials 
              from around the world and work with master artisans who have dedicated their lives to 
              perfecting their craft.
            </p>
          </div>
        </div>

        {/* Values */}
        <div className="mb-20">
          <h2 className="text-4xl font-bold text-white mb-12 text-center">Our Values</h2>
          <div className="grid grid-cols-1 md:grid-cols-3 gap-8">
            <div className="bg-gradient-to-br from-gray-900 to-black rounded-2xl border border-gold/20 p-8 text-center">
              <div className="w-20 h-20 bg-gradient-to-br from-gold to-yellow-600 rounded-full flex items-center justify-center mx-auto mb-6">
                <svg className="w-10 h-10 text-black" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z" />
                </svg>
              </div>
              <h3 className="text-2xl font-bold text-white mb-4">Quality First</h3>
              <p className="text-gray-400">
                We never compromise on quality. Every stitch, every material, every detail is carefully 
                selected and inspected to meet our exacting standards.
              </p>
            </div>

            <div className="bg-gradient-to-br from-gray-900 to-black rounded-2xl border border-gold/20 p-8 text-center">
              <div className="w-20 h-20 bg-gradient-to-br from-gold to-yellow-600 rounded-full flex items-center justify-center mx-auto mb-6">
                <svg className="w-10 h-10 text-black" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M13 10V3L4 14h7v7l9-11h-7z" />
                </svg>
              </div>
              <h3 className="text-2xl font-bold text-white mb-4">Innovation</h3>
              <p className="text-gray-400">
                We push the boundaries of design and technology, incorporating aerospace-grade materials 
                and revolutionary comfort systems.
              </p>
            </div>

            <div className="bg-gradient-to-br from-gray-900 to-black rounded-2xl border border-gold/20 p-8 text-center">
              <div className="w-20 h-20 bg-gradient-to-br from-gold to-yellow-600 rounded-full flex items-center justify-center mx-auto mb-6">
                <svg className="w-10 h-10 text-black" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 3v4M3 5h4M6 17v4m-2-2h4m5-16l2.286 6.857L21 12l-5.714 2.143L13 21l-2.286-6.857L5 12l5.714-2.143L13 3z" />
                </svg>
              </div>
              <h3 className="text-2xl font-bold text-white mb-4">Exclusivity</h3>
              <p className="text-gray-400">
                Limited production runs ensure that each Hogi owner possesses something truly special 
                and unique.
              </p>
            </div>
          </div>
        </div>

        {/* Craftsmanship */}
        <div className="mb-20">
          <div className="bg-gradient-to-br from-gray-900 to-black rounded-2xl border border-gold/20 p-12">
            <h2 className="text-3xl font-bold text-gold mb-6 text-center">Artisan Craftsmanship</h2>
            <div className="grid grid-cols-1 md:grid-cols-2 gap-8 text-gray-300">
              <div>
                <h3 className="text-xl font-bold text-white mb-3">Premium Materials</h3>
                <p className="leading-relaxed">
                  We source the finest Italian leather, aerospace-grade carbon fiber, and precious metals 
                  from trusted suppliers around the world. Each material is chosen for its exceptional 
                  quality, durability, and aesthetic appeal.
                </p>
              </div>
              <div>
                <h3 className="text-xl font-bold text-white mb-3">Master Artisans</h3>
                <p className="leading-relaxed">
                  Our team of master craftsmen brings decades of experience to every pair. Each sneaker 
                  is meticulously hand-assembled, with attention to detail that machines simply cannot 
                  replicate.
                </p>
              </div>
              <div>
                <h3 className="text-xl font-bold text-white mb-3">Limited Production</h3>
                <p className="leading-relaxed">
                  We produce only a limited number of each design, ensuring exclusivity and maintaining 
                  our uncompromising quality standards. Each pair comes with a numbered certificate of 
                  authenticity.
                </p>
              </div>
              <div>
                <h3 className="text-xl font-bold text-white mb-3">Sustainable Luxury</h3>
                <p className="leading-relaxed">
                  We are committed to sustainable practices, working with suppliers who share our values 
                  and using eco-friendly processes wherever possible without compromising on quality.
                </p>
              </div>
            </div>
          </div>
        </div>

        {/* CTA */}
        <div className="text-center">
          <h2 className="text-3xl font-bold text-white mb-6">Experience Hogi</h2>
          <p className="text-gray-400 mb-8 max-w-2xl mx-auto">
            Join the exclusive community of discerning individuals who demand the very best in luxury footwear.
          </p>
          <a
            href="/shop"
            className="inline-block px-8 py-4 bg-gradient-to-r from-gold to-yellow-600 text-black font-bold rounded-full hover:shadow-2xl hover:shadow-gold/50 transition-all duration-300"
          >
            Explore Collection
          </a>
        </div>
      </div>
    </div>
  );
}
