

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Sat Nov 25 16:29:06 2017
 */
/* Compiler settings for VCamRenderer.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IVCamRenderer,0x855DAEFD,0x6664,0x49C4,0xBB,0xAA,0x81,0x3A,0x9A,0x41,0x65,0x9E);


MIDL_DEFINE_GUID(IID, LIBID_VCamRendererLib,0x33F92BA6,0x6CC9,0x48E9,0xB7,0xDC,0xC2,0x0C,0x03,0x3F,0x89,0xD3);


MIDL_DEFINE_GUID(CLSID, CLSID_VCamRenderer,0x3D2F839E,0x1186,0x4FCE,0xB7,0x72,0xB6,0x1F,0xAE,0x1A,0xCE,0xD7);


MIDL_DEFINE_GUID(CLSID, CLSID_VCamRendererPropertyPage,0x5FC82DBE,0xC7EE,0x4443,0x98,0x4F,0x75,0xDE,0x03,0xFA,0x7C,0x3A);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



