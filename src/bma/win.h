#ifndef BMA_WIN_H_INCLUDED
#define BMA_WIN_H_INCLUDED

/*
==============================================================================
MIT License

Copyright (c) 2026 Jens Ganter-Benzing

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
==============================================================================

Wrapper around <Windows.h> to reduce the amount of stuff included and to avoid macro name clashes with other libraries.
*/

#ifndef BMA_FULL_WINDOWS_H
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
		#define BMA_UNDEF_WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
		#define NOMINMAX
		#define BMA_UNDEF_NOMINMAX
	#endif
#endif
#include <Windows.h>
/* Clean-up of temporary macro definitions */
#ifdef BMA_UNDEF_WIN32_LEAN_AND_MEAN
	#undef WIN32_LEAN_AND_MEAN
	#undef BMA_UNDEF_WIN32_LEAN_AND_MEAN
#endif
#ifdef BMA_UNDEF_NOMINMAX
	#undef NOMINMAX
	#undef BMA_UNDEF_NOMINMAX
#endif

#endif /* BMA_WIN_H_INCLUDED */
