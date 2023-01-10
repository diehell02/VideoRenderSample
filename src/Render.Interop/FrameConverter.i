/* File : FrameConverter.i */
%module Interop

%{
#include "FrameConverter.h"
%}
%apply void *VOID_INT_PTR { void * }
/* Let's just grab the original header file here */
%include "FrameConverter.h"
