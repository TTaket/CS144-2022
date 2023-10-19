#include "tcp_receiver.hh"
#include "wrapping_integers.hh"
#include <cstdint>
#include <optional>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
// class TCPSegment :
//     TCPHeader
//     Buffer
//     size_t length_in_sequence_space() const;


// TCPHeader::LENGTH = 20;  //!< [TCP](\ref rfc::rfc793) header length, not including options

// //!   0                   1                   2                   3
// //!   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// //!  |          Source Port          |       Destination Port        |
// //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// //!  |                        Sequence Number                        |
// //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// //!  |                    Acknowledgment Number                      |
// //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// //!  |  Data |           |U|A|P|R|S|F|                               |
// //!  | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
// //!  |       |           |G|K|H|T|N|N|                               |
// //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// //!  |           Checksum            |         Urgent Pointer        |
// //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// //!  |                    Options                    |    Padding    |
// //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// //!  |                             data                              |
// //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// //! ~~~

// //! \name TCP Header fields
// //!@{
// uint16_t sport = 0;         //!< source port
// uint16_t dport = 0;         //!< destination port
// WrappingInt32 seqno{0};     //!< sequence number
// WrappingInt32 ackno{0};     //!< ack number
// uint8_t doff = LENGTH / 4;  //!< data offset
// bool urg = false;           //!< urgent flag
// bool ack = false;           //!< ack flag
// bool psh = false;           //!< push flag
// bool rst = false;           //!< rst flag
// bool syn = false;           //!< syn flag
// bool fin = false;           //!< fin flag
// uint16_t win = 0;           //!< window size
// uint16_t cksum = 0;         //!< checksum
// uint16_t uptr = 0;          //!< urgent pointer
// //!@}
    
    if(SYN_FLAG == false && seg.header().syn ==false) return;
    if(SYN_FLAG == false && seg.header().syn ==true){
        //isn
        _isn = seg.header().seqno;
    }

    //在确定isn之后 对于不符合的序列号进行处理
    //序列号超过了能接受的上限的时候 忽略
    // if(unwrap(seg.header().seqno , _isn ,  _reassembler.ready_bytes_) >= _reassembler.ready_bytes_ + window_size()) return;
    // //序列号完整的小于了已经接受的ack的时候 忽略
    // if(unwrap(seg.header().seqno , _isn ,  _reassembler.ready_bytes_) + seg.length_in_sequence_space() <= _reassembler.ready_bytes_) return;

    //更新
    _reassembler.push_substring(seg.payload().copy(), unwrap(seg.header().seqno, _isn,  _reassembler.ready_bytes_), seg.header().fin);


    if(SYN_FLAG == false && seg.header().syn ==true){
        SYN_FLAG = true;
        _reassembler.insert_SYN();
    }

    if(FIN_FLAG ==false && _reassembler._output.input_ended()){
        FIN_FLAG = true;
        _reassembler.insert_FIN();
    }
}


TCPReceiver::TCPReceiver(const size_t capacity) 
    : _reassembler(capacity)
    , _capacity(capacity) 
    , _isn(0)
    , SYN_FLAG(false)
    , FIN_FLAG(false)
{}


optional<WrappingInt32> TCPReceiver::ackno() const { 
    if(SYN_FLAG == false){
        return nullopt;
    } 
    return {wrap(_reassembler.ready_bytes_ , _isn)}; 
}



size_t TCPReceiver::window_size() const{return _reassembler.output_remaincapacity();};


size_t TCPReceiver::unassembled_bytes() const { return _reassembler.unassembled_bytes(); }


    