#ifndef PARFLOWIO_PFUTIL_HPP
#define PARFLOWIO_PFUTIL_HPP
#include <cstdint>

/** Tests if the machine is little endian.
 * \return          `true` if the machine is little endian.
 */
bool isLittleEndian();

/** Tests if the machine is big endian.
 * \return          `true` if the machine is big endian.
 */
bool isBigEndian();

/** Returns `data` with the byte order reversed on little endian platforms.
 * \param   data    The data to be reversed
 * \return          `data` with the byte order reversed if the platform is little endian.
 */
uint32_t bswap32(uint32_t data);

/** Returns `data` with the byte order reversed on little endian platforms.
 * \param   data    The data to be reversed
 * \return          `data` with the byte order reversed if the platform is little endian.
 */
uint64_t bswap64(uint64_t data);

#endif //PARFLOWIO_PFUTIL_HPP
