#include "tcp_sender.hh"

#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <cstddef>
#include <cstdint>
#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.


using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _segments_out()
    , _ReadyResend()
    , _initial_retransmission_timeout(retx_timeout)
    , _stream(capacity) 
    , _next_seqno(0)
    , Resend_times(0)
    , _syn(false)
    , _fin(false)
    , Now_Time(0)
    , _checkpoint(0)
    , Window_Size(0)
    , NOW_RTO(retx_timeout)
    , timer({})
    , _bytesflight(0)
{}

uint64_t TCPSender::bytes_in_flight() const {  
    return _bytesflight;
}

void TCPSender::Send_meg(TCPSegment & msg ){
    //序列号
    msg.header().seqno = wrap(_next_seqno, _isn);
    //放入两个队列
    _segments_out.push(msg);
    _ReadyResend.push(msg);

    //更新check
    _checkpoint += msg.length_in_sequence_space();
    //更新序列号
    _next_seqno += msg.length_in_sequence_space();
    //待确认长度
    _bytesflight+= msg.length_in_sequence_space();


    //开启定时器
    if(!timer.Is_Open()) 
        timer.Start(Now_Time, NOW_RTO);
}

void TCPSender::fill_window() {
    if(_fin){
        return;
    }
    //如果是关闭状态先发送isn和SYN
    if(!_syn){
        TCPSegment SYN_PKG;
        SYN_PKG.header().syn = true;
        

        // 如果输入流是关闭的
        if(_stream.eof()){
            SYN_PKG.header().fin = true;
            _fin = true;
        }else{
            _syn = true;
        }
        Send_meg(SYN_PKG);
        return;
    }

    //正常模式
    size_t AbleSend = (Window_Size == 0) ? 1 : Window_Size;
    AbleSend = AbleSend - _bytesflight;

    while((AbleSend >0) && (!_stream.buffer_empty())){
        size_t pkg_size = min({AbleSend , _stream.buffer_size() ,TCPConfig::MAX_PAYLOAD_SIZE});
        
        //封装包裹
        TCPSegment PKG;
        PKG.payload() = _stream.read(pkg_size);

        

        //如果此时流结束了而且还有空间的话
        if(_stream.eof() &&  (AbleSend >pkg_size) ){
            PKG.header().fin = true;
            _fin = true;
        }
        //AbleSend减少
        AbleSend -= PKG.length_in_sequence_space();
        //发送包裹
        Send_meg(PKG);
    }

    //检测是否关闭 //零窗口嗅探特判 
    if(_stream.eof() && ((Window_Size > _bytesflight) || (Window_Size == 0 &&  _bytesflight ==  0)) && !_fin ){
        TCPSegment FIN_PKG;
        FIN_PKG.header().fin = true;
        //发送包裹
        Send_meg(FIN_PKG);
        _fin = true;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    //拒绝某些请求包
    if(unwrap(ackno ,_isn , _checkpoint)  > _next_seqno ) return;
    if(unwrap(ackno ,_isn , _checkpoint)  < _next_seqno - _bytesflight) return;
    
    //窗口大小转换
    this->Window_Size =  static_cast<size_t>(window_size);

    //遍历队列
    bool delflag = false;
    uint64_t ackindex = unwrap(ackno, _isn, _checkpoint);
    while(!_ReadyResend.empty()){
        auto it = _ReadyResend.front();
        uint64_t beginindex = unwrap(it.header().seqno, _isn, _checkpoint);
        uint64_t endindex = beginindex + it.length_in_sequence_space();
        if(ackindex >= endindex){
            delflag = true;
            _bytesflight -= it.length_in_sequence_space();
            _ReadyResend.pop();
        }else{
            break;
        }
    }
    if(delflag){
        //重置计时器
        Resend_times = 0;
        NOW_RTO = _initial_retransmission_timeout;
        timer.Start(Now_Time, NOW_RTO);
        if(_ReadyResend.empty()){
            timer.Close();
        }
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    Now_Time += static_cast<uint64_t>(ms_since_last_tick);

    if(timer.Is_Open() == false) return;

    //检测超时 - 如果有超时
    if(timer.Is_OverTime(Now_Time)){
        //找到最前面的 进行重传
        TCPSegment REPKG = _ReadyResend.front();
        _segments_out.push(REPKG);
        if (Window_Size > 0 || REPKG.header().syn) {
            //超时相关
            Resend_times ++;
            NOW_RTO = NOW_RTO*2;
        }
        timer.Close();
        timer.Start(Now_Time, NOW_RTO);
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { 
    return Resend_times;
}

void TCPSender::send_empty_segment() {
    _segments_out.push(TCPSegment({}));
}


TIMER::TIMER()
    : starttime(0)
    , outtime(0)
    , Isopen(false){
}
void TIMER::Start(uint64_t start_time , uint64_t out_time){
    this->starttime = start_time;
    this->outtime   = out_time;
    this->Isopen    = true; 
}
void TIMER::Close(){
    this->Isopen = false;
}
bool TIMER::Is_Open(){
    return this->Isopen;
}
bool TIMER::Is_OverTime(uint64_t nowtime){
    if(Isopen == false) return false;
    if(nowtime >= starttime +  outtime){
        return true;
    }else{
        return false;
    }
}
