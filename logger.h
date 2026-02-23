// logger.h
#pragma once
#include <iostream>

#ifdef DEBUG
  #define DBG(msg) (std::cerr << "[DEBUG] " << msg)
#else
  #define DBG(msg) ((void)0)
#endif

