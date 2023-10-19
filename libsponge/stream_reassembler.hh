#ifndef SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
#define SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH

#include "byte_stream.hh"

#include <cstdint>
#include <string>
#include <map>

//! \brief A class that assembles a series of excerpts from a byte stream (possibly out of order,
//! possibly overlapping) into an in-order byte stream.
class StreamReassembler {
  private:
    class Block{
      public:
        std::string data;
      public:
        size_t data_len;
        unsigned long index_begin;
        unsigned long index_end;
        bool Iseof;
      public:
        Block(unsigned long , std::string& , bool);
        Block();
      public:
        bool operator ==(const Block&that)const{
          return (data==that.data) && (index_begin == that.index_begin) && (Iseof == that.Iseof);
        }
    };
  public:
    // Your code here -- add private members as necessary.

    //已经准备好的字节数 
    size_t ready_bytes_;

    //正在等待被重组的字节数
    size_t unassembled_bytes_;

    //输出的字节流
    ByteStream _output;  //!< The reassembled in-order byte stream

    //重组器的大小
    size_t _capacity;    //!< The maximum number of bytes
  
    //存放结构
    std::map<unsigned long , StreamReassembler::Block>Blocks_;
  private:
    //合并两个块 b2的内容合并到b1 里面
    void merge_block(Block &b1 , Block &b2);
    
    //更新output
    void update_output(size_t index);

  public:
    //! \brief Construct a `StreamReassembler` that will store up to `capacity` bytes.
    //! \note This capacity limits both the bytes that have been reassembled,
    //! and those that have not yet been reassembled.
    StreamReassembler(const size_t capacity);

    //! \brief Receives a substring and writes any newly contiguous bytes into the stream.
    //!
    //! If accepting all the data would overflow the `capacity` of this
    //! `StreamReassembler`, then only the part of the data that fits will be
    //! accepted. If the substring is only partially accepted, then the `eof`
    //! will be disregarded.
    //!
    //! \param data the string being added
    //! \param index the index of the first byte in `data`
    //! \param eof whether or not this segment ends with the end of the stream
    void push_substring(const std::string &data, const uint64_t index, const bool eof);

    //! \name Access the reassembled byte stream
    //!@{
    const ByteStream &stream_out() const { return _output; }
    ByteStream &stream_out() { return _output; }
    //!@}

    //! The number of bytes in the substrings stored but not yet reassembled
    //!
    //! \note If the byte at a particular index has been submitted twice, it
    //! should only be counted once for the purpose of this function.
    size_t unassembled_bytes() const;

    size_t output_remaincapacity() const;

    //! \brief Is the internal state empty (other than the output stream)?
    //! \returns `true` if no substrings are waiting to be assembled
    bool empty() const;
    void insert_SYN();
    void insert_FIN();
};

#endif  // SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
