#ifndef __DATA_HEADER_ITERATOR__H__
#define __DATA_HEADER_ITERATOR__H__

#include <Data.h>

struct Data_HeaderContainer_itemIterator
{
  Data_HeaderContainer_itemIterator(Data::HeaderContainer &cnt) : 
    _cur(cnt.header().begin()),_end(cnt.header().end())
  {
  }
  
  std::map<std::string,std::string>::iterator _cur;
  std::map<std::string,std::string>::iterator _end;
};

#endif
