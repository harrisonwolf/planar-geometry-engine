// logger.h
#pragma once
#include <iostream>
#include <iomanip>

#ifdef DEBUG
  #define DBG(msg) (std::cerr << std::fixed << std::setprecision(20) << "[DEBUG] " << msg)
#else
  #define DBG(msg) ((void)0)
#endif

