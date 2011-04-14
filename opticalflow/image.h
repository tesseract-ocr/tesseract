// Copyright 2009 Google Inc. All Rights Reserved.
// Author: andrewharp@google.com (Andrew Harp)

#ifndef JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_OPTICALFLOW_IMAGE_H_
#define JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_OPTICALFLOW_IMAGE_H_

#include "optical_flow_utils.h"

// TODO(andrewharp): Make this a cast to uint32 if/when we go unsigned for
// operations.
#define ZERO 0

#ifdef SANITY_CHECKS
  #define CHECK_PIXEL(IMAGE, X, Y) {\
    CHECK((IMAGE)->validPixel((X), (Y)), \
          "CHECK_PIXEL(%d,%d) in %dx%d image.", \
          static_cast<int32>(X), static_cast<int32>(Y), \
          (IMAGE)->getWidth(), (IMAGE)->getHeight());\
  }

  #define CHECK_PIXEL_INTERP(IMAGE, X, Y) {\
    CHECK((IMAGE)->validInterpPixel((X), (Y)), \
          "CHECK_PIXEL_INTERP(%.2f, %.2f) in %dx%d image.", \
          static_cast<float32>(X), static_cast<float32>(Y), \
          (IMAGE)->getWidth(), (IMAGE)->getHeight());\
  }
#else
  #define CHECK_PIXEL(image, x, y) {}
  #define CHECK_PIXEL_INTERP(IMAGE, X, Y) {}
#endif

namespace flow {

// TODO(andrewharp): Make explicit which operations support negative numbers or
// struct/class types in image data (possibly create fast multi-dim array class
// for data where pixel arithmetic does not make sense).

// Image class optimized for working on numeric arrays as grayscale image data.
// Supports other data types as a 2D array class, so long as no pixel math
// operations are called (convolution, downsampling, etc).
template <typename T>
class Image {
 public:
  Image(const int32 width, const int32 height) :
      width_(width),
      height_(height),
      width_less_one_(width_ - 1),
      height_less_one_(height_ - 1),
      num_pixels_(width_ * height_) {
    allocate();
  }

  explicit Image(const Size& size) :
      width_(size.width),
      height_(size.height),
      width_less_one_(width_ - 1),
      height_less_one_(height_ - 1),
      num_pixels_(width_ * height_) {
    allocate();
  }

  // Constructor that creates an image from preallocated data.
  // Note: The image takes ownership of the data.
  Image(const int32 width, const int32 height, T* const image) :
      width_(width),
      height_(height),
      width_less_one_(width_ - 1),
      height_less_one_(height_ - 1),
      num_pixels_(width_ * height_) {
    image_data_ = image;
    if (image_data_ == NULL) {
      LOGE("Can't create image with NULL data!");
    }
  }

  ~Image() {
    free(image_data_);
  }

  inline int32 getWidth() const { return width_; }
  inline int32 getHeight() const { return height_; }

  // Bilinearly sample a value between pixels.
  // Values outside of the image are sampled from the nearest edge of the image.
  inline float32 getPixelInterp(const float32 x, const float32 y) const {
    // Do int32 conversion one time.
    const int32 floored_x = (int32) x;
    const int32 floored_y = (int32) y;

    // Note: it might be the case that the *_[min|max] values are clipped, and
    // these (the a b c d vals) aren't (for speed purposes), but that doesn't
    // matter. We'll just be blending the pixel with itself in that case anyway.
    const float32 b = x - floored_x;
    const float32 a = 1.0f - b;

    const float32 d = y - floored_y;
    const float32 c = 1.0f - d;

    CHECK(validInterpPixel(x, y),
          "x or y out of bounds! %.2f [0 - %d), %.2f [0 - %d)",
          x, width_less_one_, y, height_less_one_);

    const T* const pix_ptr = getPixelPtrConst(floored_x, floored_y);

// Experimental NEON acceleration... not to be turned on until it's faster.
#if FALSE
#ifdef HAVE_ARMEABI_V7A
    if (supportsNeon()) {
      // Output value:
      // a * c * p1 +
      // b * c * p2 +
      // a * d * p3 +
      // b * d * p4
      const float32x2_t ab = {a, b};
      const float32x4_t ab_c_ab_d = vcombine_f32(vmul_n_f32(ab, c),
                                                 vmul_n_f32(ab, d));

      const float32x4_t p1p2p3p4 = {pix_ptr[0], pix_ptr[1],
                                    pix_ptr[width_], pix_ptr[width_ + 1]};

      float32x4_t almost = vmulq_f32(ab_c_ab_d, p1p2p3p4);

      // Butterfly-ish sum.
      almost = vaddq_f32(vrev64q_f32(almost), almost);

      return vgetq_lane_f32(almost, 0) + vgetq_lane_f32(almost, 1);
    }
#endif
#endif

    // Get the pixel values surrounding this point.
    const T& p1 = pix_ptr[0];
    const T& p2 = pix_ptr[1];
    const T& p3 = pix_ptr[width_];
    const T& p4 = pix_ptr[width_ + 1];

    // Simple bilinear interpolation between four reference pixels.
    // If x is the value requested:
    //     a  b
    //   -------
    // c |p1 p2|
    //   |  x  |
    // d |p3 p4|
    //   -------
    return  c * ((a * p1) + (b * p2)) +
            d * ((a * p3) + (b * p4));
  }

  // Returns true iff the pixel is in the image's boundaries.
  inline bool validPixel(const int32 x, const int32 y) const {
    return inRange(x, ZERO, width_less_one_) &&
           inRange(y, ZERO, height_less_one_);
  }

  // Returns true iff the pixel is in the image's boundaries for interpolation
  // purposes.
  // TODO(andrewharp): check in interpolation follow-up change.
  inline bool validInterpPixel(const float32 x, const float32 y) const {
    // Exclusive of max because we can be more efficient if we don't handle
    // interpolating on or past the last pixel.
    return (x >= ZERO) && (x < width_less_one_) &&
           (y >= ZERO) && (y < height_less_one_);
  }

  // Safe lookup with boundary enforcement.
  inline T getPixelClipped(const int32 x, const int32 y) const {
    return getPixel(clip(x, ZERO, width_less_one_),
                    clip(y, ZERO, height_less_one_));
  }

  // Returns a const pointer to the pixel in question.
  inline const T* getPixelPtrConst(const int32 x, const int32 y) const {
    CHECK_PIXEL(this, x, y);
    return image_data_ + y * width_ + x;
  }

  // Returns a pointer to the pixel in question.
  inline T* getPixelPtr(const int32 x, const int32 y) const {
    CHECK_PIXEL(this, x, y);
    return image_data_ + y * width_ + x;
  }

  // Fast lookup without boundary enforcement.
  inline const T getPixel(const int32 x, const int32 y) const {
    CHECK_PIXEL(this, x, y);
    return image_data_[y * width_ + x];
  }

  // Fast setting without boundary enforcement.
  inline void setPixel(const int32 x, const int32 y, const T& val) {
    CHECK_PIXEL(this, x, y);
    image_data_[y * width_ + x] = val;
  }

  // Clears image to a single value.
  inline void clear(const T& val) {
    memset(image_data_, val, sizeof(*image_data_) * num_pixels_);
  }


#ifdef HAVE_ARMEABI_V7A
  // This function does the bulk of the work.
  inline void downsample32ColumnsNeon(const uint8* const original,
                                      const int32 stride,
                                      const int32 orig_x) {
    // Divide input x offset by 4 to find output offset.
    const int32 new_x = orig_x >> 2;

    // Initial offset into top row.
    const uint8* offset = original + orig_x;

    // Sum along vertical columns.
    // Process 32x4 input pixels and 8x1 output pixels per iteration.
    for (int32 new_y = 0; new_y < height_; ++new_y) {
      uint16x8_t accum1 = vdupq_n_u16(0);
      uint16x8_t accum2 = vdupq_n_u16(0);

      // Go top to bottom across the four rows of input pixels that make up
      // this output row.
      for (int32 row_num = 0; row_num < 4; ++row_num) {
        // First 16 bytes.
        {
          // Load 32 bytes of data from current offset.
          const uint8x16_t curr_data1 = vld1q_u8(offset);
          // Pairwise add and accumulate into accum vectors (16 bit to account
          // for values above 255).
          accum1 = vpadalq_u8(accum1, curr_data1);
        }

        // Second 16 bytes.
        {
          // Load 32 bytes of data from current offset.
          const uint8x16_t curr_data2 = vld1q_u8(offset + 16);
          // Pairwise add and accumulate into accum vectors (16 bit to account
          // for values above 255).
          accum2 = vpadalq_u8(accum2, curr_data2);
        }

        // Move offset down one row.
        offset += stride;
      }

      // Add and widen, then divide by 16 (number of input pixels per output
      // pixel) and narrow data from 32 bits per pixel to 16 bpp.
      const uint16x4_t tmp_pix1 = vqshrn_n_u32(vpaddlq_u16(accum1), 4);
      const uint16x4_t tmp_pix2 = vqshrn_n_u32(vpaddlq_u16(accum2), 4);

      // Combine 4x1 pixel strips into 8x1 pixel strip and narrow from
      // 16 bits to 8 bits per pixel.
      const uint8x8_t allpixels = vmovn_u16(vcombine_u16(tmp_pix1, tmp_pix2));

      // This points to the leftmost pixel of our 8 horizontally arranged
      // pixels in the destination data.
      uint8* const ptr_dst = getPixelPtr(new_x, new_y);

      // Copy all pixels from composite 8x1 vector into output strip.
      vst1_u8(ptr_dst, allpixels);
    }
  }


  // Hardware accelerated downsampling method for supported devices.
  // Requires that image size be a multiple of 16 pixels in each dimension,
  // and that downsampling be by a factor of 4.
  void downsampleAveragedNeon(const uint8* const original,
                              const int32 stride) {
    // Hardcoded to only work on 4x downsampling.
    const int32 orig_width = width_ * 4;

    // We process 32 input pixels lengthwise at a time.
    // The output per pass of this loop is an 8 wide by 1 tall pixel strip.
    for (int32 orig_x = 0; orig_x < orig_width; orig_x += 32) {
      // Push it to the left enough so that it never goes out of bounds.
      // This will result in some extra computation on the last pass on
      // devices whose frame widths are not multiples of 32.
      downsample32ColumnsNeon(original, stride, min(orig_x, orig_width - 32));
    }
  }
#endif


  // Naive downsampler that reduces image size by factor by averaging pixels in
  // blocks of size factor x factor.
  void downsampleAveraged(const T* const original, const int32 stride,
                          const int32 factor) {
#ifdef HAVE_ARMEABI_V7A
    if (supportsNeon() &&
        factor == 4 &&
        (height_ % 4) == 0) {
      downsampleAveragedNeon(original, stride);
      return;
    }
#endif

    const int32 pixels_per_block = factor * factor;

    // For every pixel in resulting image.
    for (int32 y = 0; y < height_; ++y) {
      const int32 orig_y = y * factor;
      const int32 y_bound = orig_y + factor;

      // Sum up the original pixels.
      for (int32 x = 0; x < width_; ++x) {
        const int32 orig_x = x * factor;
        const int32 x_bound = orig_x + factor;

        // Making this int32 because type U or T might overflow.
        int32 pixel_sum = 0;

        // Grab all the pixels that make up this pixel.
        for (int32 curr_y = orig_y; curr_y < y_bound; ++curr_y) {
          const T* p = original + curr_y * stride + orig_x;

          for (int32 curr_x = orig_x; curr_x < x_bound; ++curr_x) {
            pixel_sum += *p++;
          }
        }

        setPixel(x, y, pixel_sum / pixels_per_block);
      }
    }
  }

  // Naive downsampler that reduces image size by factor by averaging pixels in
  // blocks of size factor x factor.
  void downsampleAveraged(const Image<T>& original, const int32 factor) {
    downsampleAveraged(original.getPixelPtr(0, 0), original.getWidth(), factor);
  }

  // Relatively efficient downsampling of an image by a factor of two with a
  // low-pass 3x3 smoothing operation thrown in.
  void downsampleSmoothed3x3(const Image<T>& original) {
    for (int32 y = 0; y < height_; ++y) {
      const int32 orig_y = clip(2 * y, ZERO, original.height_less_one_);
      const int32 min_y = clip(orig_y - 1, ZERO, original.height_less_one_);
      const int32 max_y = clip(orig_y + 1, ZERO, original.height_less_one_);

      for (int32 x = 0; x < width_; ++x) {
        const int32 orig_x = clip(2 * x, ZERO, original.width_less_one_);
        const int32 min_x = clip(orig_x - 1, ZERO, original.width_less_one_);
        const int32 max_x = clip(orig_x + 1, ZERO, original.width_less_one_);

        // Center.
        int32 pixel_sum = original.getPixel(orig_x, orig_y) * 4;

        // Sides.
        pixel_sum += (original.getPixel(max_x, orig_y) +
                      original.getPixel(min_x, orig_y) +
                      original.getPixel(orig_x, max_y) +
                      original.getPixel(orig_x, min_y)) * 2;

        // Diagonals.
        pixel_sum += (original.getPixel(max_x, max_y) +
                      original.getPixel(min_x, max_y) +
                      original.getPixel(max_x, min_y) +
                      original.getPixel(min_x, min_y));

        const int32 pixel_val = pixel_sum>>4;  // 16

        //LOGV("Setting %d,%d to %d", col, row, pixel_val);

        setPixel(x, y, pixel_val);
      }
    }
  }

  // Relatively efficient downsampling of an image by a factor of two with a
  // low-pass 5x5 smoothing operation thrown in.
  void downsampleSmoothed5x5(const Image<T>& original) {
    const int32 max_x = original.width_less_one_;
    const int32 max_y = original.height_less_one_;

    // The JY Bouget paper on Lucas-Kanade recommends a
    // [1/16 1/4 3/8 1/4 1/16]^2 filter.
    // This works out to a [1 4 6 4 1]^2 / 256 array, precomputed below.
    static const int32 window_radius = 2;
    static const int32 window_size = window_radius*2 + 1;
    static const int32 window_weights[] = {1, 4, 6, 4,1,  // 16 +
                                           4,16,24,16,4,  // 64 +
                                           6,24,36,24,6,  // 96 +
                                           4,16,24,16,4,  // 64 +
                                           1, 4, 6, 4,1}; // 16 = 256

    // We'll multiply and sum with the the whole numbers first, then divide by
    // the total weight to normalize at the last moment.
    for (int32 y = 0; y < height_; ++y) {
      for (int32 x = 0; x < width_; ++x) {
        int32 pixel_sum = 0;

        const int32* w = window_weights;
        const int32 start_x = clip((x<<1) - window_radius, ZERO, max_x);

        // Clip the boundaries to the size of the image.
        for (int32 window_y = 0; window_y < window_size; ++window_y) {
          const int32 start_y =
              clip((y<<1) - window_radius + window_y, ZERO, max_y);

          const T* p = original.getPixelPtrConst(start_x, start_y);

          for (int32 window_x = 0; window_x < window_size; ++window_x) {
            pixel_sum +=  *p++ * *w++;
          }
        }

        // Conversion to type T will happen here after shifting right 8 bits to
        // divide by 256.
        setPixel(x, y, pixel_sum >> 8);
      }
    }
  }

  // Optimized Scharr filter on a single pixel in the X direction.
  // Scharr filters are like central-difference operators, but have more
  // rotational symmetry in their response because they also consider the
  // diagonal neighbors.
  template <typename U>
  inline T scharrPixelX(const Image<U>& original,
                        const int32 center_x, const int32 center_y) const {
    const int32 min_x = clip(center_x - 1, ZERO, original.width_less_one_);
    const int32 max_x = clip(center_x + 1, ZERO, original.width_less_one_);
    const int32 min_y = clip(center_y - 1, ZERO, original.height_less_one_);
    const int32 max_y = clip(center_y + 1, ZERO, original.height_less_one_);

    // Convolution loop unrolled for performance...
    return (3 * (original.getPixel(max_x, min_y)
                 + original.getPixel(max_x, max_y)
                 - original.getPixel(min_x, min_y)
                 - original.getPixel(min_x, min_y))
            + 10 * (original.getPixel(max_x, center_y)
                    - original.getPixel(min_x, center_y))) / 32;
  }

  // Optimized Scharr filter on a single pixel in the X direction.
  // Scharr filters are like central-difference operators, but have more
  // rotational symmetry in their response because they also consider the
  // diagonal neighbors.
  template <typename U>
  inline T scharrPixelY(const Image<U>& original,
                        const int32 center_x, const int32 center_y) const {
    const int32 min_x = clip(center_x - 1, 0, original.width_less_one_);
    const int32 max_x = clip(center_x + 1, 0, original.width_less_one_);
    const int32 min_y = clip(center_y - 1, 0, original.height_less_one_);
    const int32 max_y = clip(center_y + 1, 0, original.height_less_one_);

    // Convolution loop unrolled for performance...
    return (3 * (original.getPixel(min_x, max_y)
                 + original.getPixel(max_x, max_y)
                 - original.getPixel(min_x, min_y)
                 - original.getPixel(max_x, min_y))
            + 10 * (original.getPixel(center_x, max_y)
                    - original.getPixel(center_x, min_y))) / 32;
  }

  // Convolve the image with a Scharr filter in the X direction.
  // Much faster than an equivalent generic convolution.
  template <typename U>
  inline void scharrX(const Image<U>& original) {
    for (int32 y = 0; y < height_; ++y) {
      for (int32 x = 0; x < width_; ++x) {
        setPixel(x, y, scharrPixelX(original, x, y));
      }
    }
  }

  // Convolve the image with a Scharr filter in the Y direction.
  // Much faster than an equivalent generic convolution.
  template <typename U>
  inline void scharrY(const Image<U>& original) {
    for (int32 y = 0; y < height_; ++y) {
      for (int32 x = 0; x < width_; ++x) {
        setPixel(x, y, scharrPixelY(original, x, y));
      }
    }
  }

  static inline T halfDiff(int32 first, int32 second) {
    return (second - first) / 2;
  }

  template <typename U>
  void derivativeX(const Image<U>& original) {
    for (int32 y = 0; y < height_; ++y) {
      T* const dest_row = getPixelPtr(0, y);
      const U* const source_row = original.getPixelPtrConst(0, y);

      // Compute first pixel.
      dest_row[0] = halfDiff(source_row[0], source_row[1]);

      // Last pixel.
      dest_row[width_less_one_] = halfDiff(source_row[width_less_one_ - 1],
                                           source_row[width_less_one_]);

      // All the pixels in between.
      const U* const source_prev_pixel = source_row - 1;
      const U* const source_next_pixel = source_row + 1;
      for (int32 x = 1; x < width_less_one_; ++x) {
        dest_row[x] = halfDiff(source_prev_pixel[x], source_next_pixel[x]);
      }
    }
  }

  template <typename U>
  void derivativeY(const Image<U>& original) {
    for (int32 y = 0; y < height_; ++y) {
      T* const dest_row = getPixelPtr(0, y);

      const U* const source_prev_pixel =
          original.getPixelPtrConst(0, max(0, y - 1));

      const U* const source_next_pixel =
          original.getPixelPtrConst(0, min(height_less_one_, y + 1));

      for (int32 x = 0; x < width_; ++x) {
        dest_row[x] = halfDiff(source_prev_pixel[x], source_next_pixel[x]);
      }
    }
  }

  // Generic function for convolving pixel with 3x3 filter.
  // Filter pixels should be in row major order.
  template <typename U>
  inline T convolvePixel3x3(const Image<U>& original,
                            const int32* const filter,
                            const int32 center_x, const int32 center_y,
                            const int32 total) const {
    int32 sum = 0;
    for (int32 filter_y = 0; filter_y < 3; ++filter_y) {
      const int32 y = clip(center_y - 1 + filter_y, 0, original.getHeight());
      for (int32 filter_x = 0; filter_x < 3; ++filter_x) {
        const int32 x = clip(center_x - 1 + filter_x, 0, original.getWidth());
        sum += original.getPixel(x, y) * filter[filter_y * 3 + filter_x];
      }
    }
    return sum / total;
  }

  // Generic function for convolving an image with a 3x3 filter.
  // TODO(andrewharp): Generalize this for any size filter.
  template <typename U>
  inline void convolve3x3(const Image<U>& original,
                          const int32* const filter) {
    int32 sum = 0;
    for (int32 i = 0; i < 9; ++i) {
      sum += abs(filter[i]);
    }
    for (int32 y = 0; y < height_; ++y) {
      for (int32 x = 0; x < width_; ++x) {
        setPixel(x, y, convolvePixel3x3(original, filter, x, y, sum));
      }
    }
  }

  // Load this image's data from a data array. The data at pixels is assumed to
  // have dimensions equivalent to this image's dimensions * factor.
  inline void fromArray(const T* const pixels, const int32 stride,
                        const int32 factor) {
    if (factor == 1) {
      // If not subsampling, memcpy per line should be faster.
      memcpy(this->image_data_, pixels, num_pixels_ * sizeof(T));
      return;
    }

    downsampleAveraged(pixels, stride, factor);
  }

  // Copy the image back out to an appropriately sized data array.
  inline void toArray(T* const pixels) const {
    // If not subsampling, memcpy should be faster.
    memcpy(pixels, this->image_data_, num_pixels_ * sizeof(T));
  }

 private:
  inline void allocate() {
    image_data_ = (T*)malloc(num_pixels_ * sizeof(T));
    if (image_data_ == NULL) {
      LOGE("Couldn't allocate image data!");
    }
  }

  T* image_data_;
  int32 width_;
  int32 height_;

 public:
  // Precompute these for efficiency's sake as they're used by a lot of
  // clipping code and loop code.
  // TODO(andrewharp): make these only accessible by other Images.
  int32 width_less_one_;
  int32 height_less_one_;
  int32 num_pixels_;
};


// Create a pyramid of downsampled images. The first level of the pyramid is the
// original image.
inline void computeSmoothedPyramid(const Image<uint8>& frame,
                                   const int32 num_levels,
                                   Image<uint8>** const pyramid) {
  // TODO(andrewharp): Find a const correct solution to this...
  // Maybe make an pyramid class with the first level of this pyramid as a
  // separate pointer?

  // Cast away const, but we're not going to hurt it, honest!
  pyramid[0] = const_cast<Image<uint8>*>(&frame);

  for (int32 l = 1; l < num_levels; ++l) {
    pyramid[l]->downsampleSmoothed3x3(*pyramid[l - 1]);
  }
}


// Create a spatial derivative pyramid based on a downsampled pyramid.
inline void computeSpatialPyramid(const Image<uint8>** const pyramid,
                                  const int32 num_levels,
                                  Image<int32>** pyramid_x,
                                  Image<int32>** pyramid_y) {
  for (int32 l = 0; l < num_levels; ++l) {
    const Image<uint8>& frame = *pyramid[l];

    // Fast convolutions to find spatial derivatives.
    pyramid_x[l]->derivativeX(frame);
    pyramid_y[l]->derivativeY(frame);
  }
}


// Marks a circle of a given radius on the boolean image.
// If the center spot is already marked, don't do anything and return false.
// Otherwise, mark everything in range true and return true.
template <typename U>
inline static bool markImage(const int32 x, const int32 y,
                             const int32 radius,
                             Image<U>* const img) {
  if (img->getPixel(x, y)) {
    // Already claimed, sorry.
    return false;
  }

  const int32 squared_radius = square(radius);

  for (int32 d_y = 0; d_y < radius; ++d_y) {
    const int32 squared_y_dist = square(d_y);

    const int32 min_y = y > d_y ? y - d_y : ZERO;
    const int32 max_y = min(y + d_y, img->height_less_one_);

    for (int32 d_x = 0; d_x < radius; ++d_x) {
      if (squared_y_dist + square(d_x) <= squared_radius) {
        const int32 min_x = x > d_x ? x - d_x : ZERO;
        const int32 max_x = min(x + d_x, img->width_less_one_);

        // Mark all four quadrants.
        img->setPixel(max_x, max_y, true);
        img->setPixel(max_x, min_y, true);
        img->setPixel(min_x, max_y, true);
        img->setPixel(min_x, min_y, true);
      } else {
        // Once we're too far out, we're not coming back in.
        break;
      }
    }
  }
  return true;
}


// Puts the image gradient matrix about a pixel into the 2x2 float array G.
// vals_x should be an array of the window x gradient values, whose indices
// can be in any order but are parallel to the vals_y entries.
// See http://robots.stanford.edu/cs223b04/algo_tracking.pdf for more details.
inline void calculateG(const float32* const vals_x, const float32* const vals_y,
                       const int32 num_vals, float* const G) {
  // Defined here because we want to keep track of how many values were
  // processed by NEON, so that we can finish off the remainder the normal
  // way.
  int32 i = 0;

#ifdef HAVE_ARMEABI_V7A
  if (supportsNeon()) {
    const float32_t* const arm_vals_x = (const float32_t*) vals_x;
    const float32_t* const arm_vals_y = (const float32_t*) vals_y;

    // Running sums.
    float32x4_t xx = vdupq_n_f32(0.0f);
    float32x4_t xy = vdupq_n_f32(0.0f);
    float32x4_t yy = vdupq_n_f32(0.0f);

    // Maximum index we can load 4 consecutive values from.
    // e.g. if there are 81 values, our last full pass can be from index 77:
    // 81-4=>77 (77, 78, 79, 80)
    const int32 max_i = num_vals - 4;

    // Process values 4 at a time, accumulating the sums of
    // the pixel-wise x*x, x*y, and y*y values.
    for (; i <= max_i; i += 4) {
      // Load xs
      float32x4_t x = vld1q_f32(arm_vals_x + i);

      // Multiply x*x and accumulate.
      xx = vmlaq_f32(xx, x, x);

      // Load ys
      float32x4_t y = vld1q_f32(arm_vals_y + i);

      // Multiply x*y and accumulate.
      xy = vmlaq_f32(xy, x, y);

      // Multiply y*y and accumulate.
      yy = vmlaq_f32(yy, y, y);
    }

    static float32_t xx_vals[4];
    static float32_t xy_vals[4];
    static float32_t yy_vals[4];

    vst1q_f32(xx_vals, xx);
    vst1q_f32(xy_vals, xy);
    vst1q_f32(yy_vals, yy);

    // Accumulated values are store in sets of 4, we have to manually add
    // the last bits together.
    for (int32 j = 0; j < 4; ++j) {
      G[0] += xx_vals[j];
      G[1] += xy_vals[j];
      G[3] += yy_vals[j];
    }
  }
#endif
  // Non-accelerated version, also finishes off last few values (< 4) from
  // above.
  for (; i < num_vals; ++i) {
    G[0] += square(vals_x[i]);
    G[1] += vals_x[i] * vals_y[i];
    G[3] += square(vals_y[i]);
  }

  // The matrix is symmetric, so this is a given.
  G[2] = G[1];
}


// Puts the image gradient matrix about a pixel into the 2x2 float array G.
// Looks up interpolated pixels, then calls above method for implementation.
inline void calculateG(const int window_size,
                       const float32 center_x, const float center_y,
                       const Image<int32>& I_x, const Image<int32>& I_y,
                       float* const G) {
  CHECK(I_x.validPixel(center_x, center_y), "Problem in calculateG!");

  // Hardcoded to allow for a max window radius of 5 (9 pixels x 9 pixels).
  static const int kMaxWindowRadius = 5;
  CHECK(window_size <= kMaxWindowRadius,
        "Window %d > %d!", window_size, kMaxWindowRadius);

  // Diameter of window is 2 * radius + 1 for center pixel.
  static const int kWindowBufferSize =
      (kMaxWindowRadius * 2 + 1) * (kMaxWindowRadius * 2 + 1);

  // Preallocate buffers statically for efficiency.
  static float32 vals_x[kWindowBufferSize];
  static float32 vals_y[kWindowBufferSize];

  int32 num_vals = 0;

  for (int32 win_x = -window_size; win_x <= window_size; ++win_x) {
    for (int32 win_y = -window_size; win_y <= window_size; ++win_y) {
      vals_x[num_vals] = I_x.getPixelInterp(center_x + win_x,
                                            center_y + win_y);
      vals_y[num_vals] = I_y.getPixelInterp(center_x + win_x,
                                            center_y + win_y);
      ++num_vals;
    }
  }
  calculateG(vals_x, vals_y, num_vals, G);
}

}  // namespace flow

#endif  // JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_OPTICALFLOW_IMAGE_H_
