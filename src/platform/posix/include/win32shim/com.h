#ifndef WIN32SHIM_COM_H_
#define WIN32SHIM_COM_H_
#include <win32shim/wintypes.h>

// COM basics: interfaces are abstract C++ structs with pure virtual methods.

#define interface struct
#define STDMETHOD(method) virtual HRESULT method
#define STDMETHOD_(type, method) virtual type method
#define STDMETHODIMP HRESULT
#define STDMETHODIMP_(type) type
#define STDAPI extern "C" HRESULT
#define STDAPI_(type) extern "C" type
#define PURE = 0
#define THIS_
#define THIS void
#define DECLARE_INTERFACE(iface) interface iface
#define DECLARE_INTERFACE_(iface, base) interface iface : public base
#define BEGIN_INTERFACE
#define END_INTERFACE

#define CLSCTX_INPROC_SERVER 0x1
#define REGDB_E_CLASSNOTREG ((HRESULT)0x80040154)
#define REGDB_E_IIDNOTREG ((HRESULT)0x80040155)
#define CO_E_NOTINITIALIZED ((HRESULT)0x800401F0)

DEFINE_GUID(IID_IUnknown, 0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
DEFINE_GUID(GUID_NULL, 0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

interface IUnknown {
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)() PURE;
    STDMETHOD_(ULONG, Release)() PURE;
};
typedef IUnknown* LPUNKNOWN;

HRESULT CoInitialize(LPVOID pvReserved);
void CoUninitialize();
HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv);

#endif /* WIN32SHIM_COM_H_ */
