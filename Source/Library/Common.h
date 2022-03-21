﻿/*+===================================================================
  File:      COMMON.H

  Summary:   Common header file that contains common header files and
			 macros used for the Library project of Game Graphics
			 Programming course.

  Functions:

  © 2022 Kyung Hee University
===================================================================+*/
#pragma once

#ifndef  UNICODE
#define UNICODE
#endif // ! UNICODE

#include <windows.h>

#include <d3d11_4.h>
#include <directxcolors.h>

#include "Resource.h"

constexpr LPCWSTR PSZ_COURSE_TITLE = L"Game Graphics Programming Lab01: Direct3D 11 Basics";

using namespace DirectX;

#include <wrl.h>
using namespace Microsoft::WRL;

#pragma region Lab00
///*+===================================================================
//  File:      COMMON.H
//
//  Summary:   Common header file that contains common header files and
//			 macros used for the Library project of Game Graphics
//			 Programming course.
//
//  Functions:
//
//  © 2022 Kyung Hee University
//===================================================================+*/
//#pragma once
//
//#ifndef  UNICODE
//#define UNICODE
//#endif // ! UNICODE
//
//#include <windows.h>
#pragma endregion