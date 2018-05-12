#pragma once

#include <Windows.h>
#include <tchar.h>

// Windows Media SDK
#include <dshow.h>
#include <Wmsdk.h>
#include <wmsdkidl.h>
#include <dshowasf.h>
#include <wmsysprf.h>
#include "dshow\baseclasses\qedit.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x) { x->Release(); x = NULL; }
#endif
