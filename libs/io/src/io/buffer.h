#ifndef ECO_IO_BUFFER
#define ECO_IO_BUFFER

#include <cstddef>

namespace eco {
namespace io {
struct Buffer {
  // MANIPULATORS

  /**
   * Return a pointer to the data array. The size of the array is at least as
   * specified by `sizeInBytes()`.
   */
  virtual char *data() noexcept = 0;

  // ACCESSORS

  /**
   * Return a const pointer to the data array. The size of the array is at least
   * as specified by `sizeInBytes()`.
   */
  virtual char const *data() const noexcept = 0;

  /**
   * Return the size of the data array inside this buffer.
   */
  virtual std::size_t sizeInBytes() const noexcept = 0;
};

/**
 * Buffer that contains an array of a static size.
 */
template <std::size_t N> class ArrayBuffer : public Buffer {
  std::array<char, N> d_data;

public:
  char *data() noexcept override { return d_data.data(); }
  char const *data() noexcept const override { return d_data.data(); }
  std::size_t sizeInBytes() const noexcept override { return N; }
};
} // namespace io
} // namespace eco

#endif // ECO_IO_BUFFER