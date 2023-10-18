#include "byte_stream.hh"
#include <cstddef>
#include <cstring>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : capacity_(capacity)
    , have_written_(0)
    , have_read_(0)
    , Buffer_(capacity)
    , is_end_(false)
    , is_error_(false)
{}


inline unsigned long ByteStream::get_pos_written_( size_t off = 0) const{
    return (have_written_+off)%capacity_;
}
inline unsigned long ByteStream::get_pos_read( size_t off = 0) const{
    return (have_read_+off)%capacity_;
}
//超过当前容量的字符会被抛弃
size_t ByteStream::write(const string &data) {
    //计算出能写入的字符数量 - 判断能不能都写进去 
    size_t AbleToAdd = min(remaining_capacity(),data.length());
    size_t i = 0;
    while(i < AbleToAdd){
        Buffer_[get_pos_written_()] = data[i];
        have_written_++;
        i++;
    }
    return AbleToAdd;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string ret;ret.resize(len);
    size_t i = 0;
    for(;i<len;i++){
        ret[i] = Buffer_[get_pos_read(i)];
    }
    return ret;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t del_len = min(len , buffer_size());
    have_read_+=del_len;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    size_t ableread = min(len , bytes_written() - bytes_read());
    string ret = peek_output(ableread);
    pop_output(ableread);
    return ret;
}

void ByteStream::end_input() {
    is_end_ = true;
}

bool ByteStream::input_ended() const {
    return is_end_;
}

size_t ByteStream::buffer_size() const {
    return have_written_ - have_read_;
}

bool ByteStream::buffer_empty() const { 
    return (have_written_ == have_read_);
}

bool ByteStream::eof() const { 
    return (is_end_ && (have_written_ == have_read_));
}

size_t ByteStream::bytes_written() const { return have_written_; }

size_t ByteStream::bytes_read() const { return have_read_; }

size_t ByteStream::remaining_capacity() const { return (capacity_ - (have_written_ - have_read_)); }

void ByteStream::set_error() { is_error_ = true; }

bool ByteStream::error() const{return is_error_;}
