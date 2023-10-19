#include "stream_reassembler.hh"
#include <algorithm>
#include <cstddef>
#include <string>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

using namespace std;

StreamReassembler::Block::Block(unsigned long index , std::string & strdata , bool eof)
    : data(strdata)
    , data_len(strdata.length())
    , index_begin(index)
    , index_end(index + strdata.length())
    , Iseof(eof)
{}

StreamReassembler::Block::Block()
    : data({})
    , data_len(0)
    , index_begin(0)
    , index_end(0)
    , Iseof(false)
{};


StreamReassembler::StreamReassembler(const size_t capacity) 
    :ready_bytes_(0)
    ,unassembled_bytes_(0)
    ,_output(capacity)
    ,_capacity(capacity)
    ,Blocks_({})
{}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    //如果字符串被裁剪（后边被裁剪 那么eof失效）
    bool flag_dealeof =false;
    //对块进行处理 - 删除前缀
    if(index + data.length() < ready_bytes_){
        //过期的信息 删掉
        return;
    }
    //对块进行处理 - 删除超过内存后缀
    if(index > (ready_bytes_ +_output.remaining_capacity()) ){
        //超过内存的信息 删掉
        return;
    }
    std::string tmpdata = data;
    std::size_t tmpindex = index;
    //计算出合法储存大小
    if(tmpindex < ready_bytes_){
        size_t dellen = ready_bytes_ - index;
        tmpdata = tmpdata.substr(dellen);
        tmpindex = ready_bytes_;
    }
    if(tmpindex + tmpdata.length() > ready_bytes_ + _output.remaining_capacity()){
        tmpdata = tmpdata.substr( 0 , ready_bytes_ +_output.remaining_capacity()  - tmpindex );
        flag_dealeof = true;
    }

    //无意义空串
    if((tmpdata.length() == 0 )&& (!eof && !flag_dealeof)){
        return;
    }

    Block newblock(tmpindex , tmpdata , eof&&(!flag_dealeof));
    //去重 
    if (Blocks_.find(tmpindex) != Blocks_.end())
    {
        if(Blocks_[tmpindex] == newblock){
            return;
        }
    }

    //数据处理 
    //查询他左侧第一个串 看看需不需要合并
    auto it = Blocks_.lower_bound(tmpindex);
    if(it != Blocks_.begin()){
        it--;
        Block beforeNode = it->second;
        if(it->second.index_end > tmpindex){
        unassembled_bytes_-= beforeNode.data_len;
        merge_block(newblock,beforeNode);
        Blocks_.erase(it);
        }
    }
    //查询大于等于它索引的串 看看需不需要合并
    it = Blocks_.lower_bound(tmpindex);
    while((it != Blocks_.end() )){
        Block afterNode = it->second;
        if((afterNode.index_begin < newblock.index_end)){
            unassembled_bytes_ -= afterNode.data_len;
            merge_block(newblock , afterNode);
            it = Blocks_.erase(it);
        }else{
            break;
        }
    }

  
  Blocks_[newblock.index_begin] = newblock;
  unassembled_bytes_ += newblock.data_len;

  //判断是否可以加入到准备好的队列;
  while((Blocks_.find(ready_bytes_)!=Blocks_.end()) && (!_output.eof())){
    update_output(ready_bytes_ );
  }
}
void StreamReassembler::update_output(size_t index){
    string tmpstr(Blocks_[index].data);
    //取出这个串并且加到output里面
    _output.write(tmpstr);

    //扩展已经准备的长度
    ready_bytes_ += tmpstr.length();
    unassembled_bytes_ -= tmpstr.length();

    //如果是最后一个子串 关闭流
    if(Blocks_[index].Iseof){
        _output.end_input();
    }

    //删除这个节点 
    Blocks_.erase(Blocks_.find(index));
}

size_t StreamReassembler::unassembled_bytes() const { return unassembled_bytes_; }

bool StreamReassembler::empty() const { return (unassembled_bytes_== 0); }


//void StreamReassembler::merge_block(StreamReassembler::Block &b1 , StreamReassembler::Block &b2){};

void StreamReassembler::merge_block (Block &BN1  ,Block &BN2  ){
  if(BN1 == BN2){
    return;
  }
  string tmpdata = "";
  //data处理 
  //如果包含关系 
  if(BN1.index_begin <= BN2.index_begin && BN1.index_end >= BN2.index_end){//如果包含关系 
    tmpdata = BN1.data;
  }else if(BN2.index_begin <= BN1.index_begin && BN2.index_end >= BN1.index_end){//如果包含关系 
    tmpdata = BN2.data;
  }else{ //非包含关系 
    if(BN2.index_begin <= BN1.index_begin && BN2.index_end <= BN1.index_end ){
      swap(BN1.data , BN2.data);
      swap(BN1.data_len , BN2.data_len);
      swap(BN1.index_end , BN2.index_end);
      swap(BN1.index_begin , BN2.index_begin);
    } 
    
    tmpdata = BN1.data;
    tmpdata += BN2.data.substr(BN1.index_end - BN2.index_begin);
  }

  BN1.index_begin = min(BN1.index_begin  , BN2.index_begin);
  BN1.index_end = max(BN1.index_end  , BN2.index_end);
  BN1.Iseof = BN1.Iseof || BN2.Iseof;
  BN1.data_len = BN1.index_end - BN1.index_begin;
  BN1.data = tmpdata;
}

size_t StreamReassembler::output_remaincapacity() const{
    return _output.remaining_capacity();
}


void StreamReassembler::insert_SYN(){ready_bytes_++;}
void StreamReassembler::insert_FIN(){ready_bytes_++;}