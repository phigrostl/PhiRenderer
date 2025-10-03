#pragma once

#ifdef DEBUG
#define ASSERT(x, ...) { if(!(x)) { __debugbreak(); } }
#else
#define ASSERT(x, ...)
#endif
