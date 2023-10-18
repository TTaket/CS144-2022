#include "wrapping_integers.hh"
#include <cstdint>

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.


using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    return WrappingInt32{(static_cast<uint32_t>(n % static_cast<uint64_t>UINT32_MAX) + isn.raw_value()) % UINT32_MAX};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t addval = (static_cast<uint64_t>UINT32_MAX + static_cast<uint64_t>(n.raw_value()) - static_cast<uint64_t>(isn.raw_value()) )% static_cast<uint64_t>UINT32_MAX;
    uint64_t l = 0;
    uint64_t r = static_cast<uint64_t>UINT32_MAX;
    while(l < r-1){
        uint64_t mid = (l + r )/2;
        if(mid * static_cast<uint64_t>UINT32_MAX + addval <= checkpoint) l=mid;
        else r =mid;
    }
    //特判
    if(r== static_cast<uint64_t>UINT32_MAX){
        return addval + l * static_cast<uint64_t>UINT32_MAX;
    }

    uint64_t val1 = addval + l * static_cast<uint64_t>UINT32_MAX;
    uint64_t val2 = addval + r * static_cast<uint64_t>UINT32_MAX;
    if(checkpoint - val1 < val2 - checkpoint){
        return val1;
    }else{
        return val2;
    }
}
