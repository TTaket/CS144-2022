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
    uint64_t baselen = (static_cast<uint64_t>( static_cast<uint64_t>(1) << 32 ) );
    uint32_t ret_raw_value_ = ((n + isn.raw_value()) %baselen);
    return WrappingInt32 { ret_raw_value_ };
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
  uint64_t baselen = (static_cast<uint64_t>( static_cast<uint64_t>(1) << 32 ) );
  uint64_t checkval = (static_cast<uint64_t>( static_cast<uint64_t>(1) << 31 ) );
  uint64_t modlen = (n.raw_value() - isn.raw_value() + baselen)%baselen;    //取mod 后的长度 也是最小的真实距离
  uint64_t retval = 0;
  if(int64_t(checkpoint)/int64_t(baselen) == 0 ){
    retval =  modlen ;
  }else{
    retval = (uint64_t(int64_t(checkpoint)/int64_t(baselen)) - 1) * baselen + modlen;
  }
  while((retval < checkpoint)){
    if((checkpoint - retval) > checkval)
      retval+=baselen;
    else
      break;
  }
  return retval ;
}
