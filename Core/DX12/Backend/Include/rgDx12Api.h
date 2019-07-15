#pragma once

#ifdef RGA_API_EXPORTS
#define RGA_API __declspec(dllexport)
#else
#define RGA_API __declspec(dllimport)
#endif
