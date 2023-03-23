// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <stdio.h>

#include "SurfaceQueue.h"

#include "d3d9.h"

#include <D3D10_1.h>
//#include <d3dx10.h>

#include <d3d9.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d2d1.h>


#define IFC(x) { hr = (x); if (FAILED(hr)) { goto Cleanup; }}
#define IFC2(hr) { if (FAILED(hr)) { goto Cleanup; }}
#define IFF(hr) { if (FAILED(hr)) return false; }
#define ReleaseInterface(x) { if (NULL != x) { x->Release(); x = NULL; }}
#define SAFE_RELEASE(punk)  { if ((punk) != nullptr) { (punk)->Release(); (punk) = nullptr; }}
