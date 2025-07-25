// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// METASIG.H
//

//
// All literal MetaData signatures should be defined here.
//


// Generic sig's based on types.

// All sigs are alphabetized by the signature string and given a canonical name.  Do not
// give them "meaningful" names because we want to share them aggressively.  Do not add
// duplicates!

// The canonical form:
//
//   <what>(<type>*, <name>*)
//
//   where <what> is:
//
//         Fld  -- field
//         IM   -- instance method (HASTHIS == TRUE)
//         SM   -- static method
//
//   and <name> -- <type> is:
//
//   a  -- Arr  -- array
//   P  -- Ptr  -- a pointer
//   r  -- Ref  -- a byref
//         Ret  -- indicates function return type
//
//   b  -- Byte -- (unsigned) byte
//   u  -- Char -- character (2 byte unsigned unicode)
//   d  -- Dbl  -- double
//   f  -- Flt  -- float
//   i  -- Int  -- integer
//   K  -- UInt  -- unsigned integer
//   I  -- IntPtr -- agnostic integer
//   U  -- UIntPtr -- agnostic unsigned integer
//   l  -- Long -- long integer
//   L  -- ULong -- unsigned long integer
//   h  -- Shrt -- short integer
//   H  -- UShrt -- unsigned short integer
//   v  -- Void -- Void
//   B  -- SByt -- signed byte
//   F  -- Bool -- boolean
//   j  -- Obj  -- System.Object
//   s  -- Str  -- System.String
//   C  --      -- class
//   g  --      -- struct
//   T  -- TypedReference -- TypedReference
//   G  --      -- Generic type variable
//   M  --      -- Generic method variable
//   GI --      -- Generic type instantiation
//   Q          -- modreq

//#DEFINE_METASIG
// Use DEFINE_METASIG for signatures that does not reference other types
// Use DEFINE_METASIG_T for signatures that reference other types (contains C or g)


// This part of the file is included multiple times with different macro definitions
// to generate the hardcoded metasigs.


// SM, IM and Fld macros have often a very similar definitions. METASIG_BODY is
// a helper to avoid these redundant SM, IM and Fld definitions.
#ifdef METASIG_BODY
#ifndef DEFINE_METASIG
// See code:#DEFINE_METASIG
#define DEFINE_METASIG(body)            body
#endif
#define SM(varname, args, retval)       METASIG_BODY( SM_ ## varname, args retval )
#define IM(varname, args, retval)       METASIG_BODY( IM_ ## varname, args retval )
#define GM(varname, conv, n, args, retval) METASIG_BODY( GM_ ## varname, args retval )
#define Fld(varname, val)               METASIG_BODY( Fld_ ## varname, val )
#endif


#ifdef DEFINE_METASIG

// Use default if undefined
// See code:#DEFINE_METASIG
#ifndef DEFINE_METASIG_T
#define DEFINE_METASIG_T(body) DEFINE_METASIG(body)
#endif

// One letter shortcuts are defined for all types that can occur in the signature.
// The shortcuts are defined indirectly through METASIG_ATOM. METASIG_ATOM is
// designed to control whether to generate the initializer for the signature or
// just compute the size of the signature.

#define b METASIG_ATOM(ELEMENT_TYPE_U1)
#define u METASIG_ATOM(ELEMENT_TYPE_CHAR)
#define d METASIG_ATOM(ELEMENT_TYPE_R8)
#define f METASIG_ATOM(ELEMENT_TYPE_R4)
#define i METASIG_ATOM(ELEMENT_TYPE_I4)
#define K METASIG_ATOM(ELEMENT_TYPE_U4)
#define I METASIG_ATOM(ELEMENT_TYPE_I)
#define U METASIG_ATOM(ELEMENT_TYPE_U)
#define l METASIG_ATOM(ELEMENT_TYPE_I8)
#define L METASIG_ATOM(ELEMENT_TYPE_U8)
#define h METASIG_ATOM(ELEMENT_TYPE_I2)
#define H METASIG_ATOM(ELEMENT_TYPE_U2)
#define v METASIG_ATOM(ELEMENT_TYPE_VOID)
#define B METASIG_ATOM(ELEMENT_TYPE_I1)
#define F METASIG_ATOM(ELEMENT_TYPE_BOOLEAN)
#define j METASIG_ATOM(ELEMENT_TYPE_OBJECT)
#define s METASIG_ATOM(ELEMENT_TYPE_STRING)
#define T METASIG_ATOM(ELEMENT_TYPE_TYPEDBYREF)


// METASIG_RECURSE controls whether to recurse into the complex types
// in the macro expansion. METASIG_RECURSE is designed to control
// whether to compute the byte size of the signature or just compute
// the number of arguments in the signature.

#if METASIG_RECURSE

#define a(x) METASIG_ATOM(ELEMENT_TYPE_SZARRAY) x
#define P(x) METASIG_ATOM(ELEMENT_TYPE_PTR) x
#define r(x) METASIG_ATOM(ELEMENT_TYPE_BYREF) x

#define G(n) METASIG_ATOM(ELEMENT_TYPE_VAR) METASIG_ATOM(n)
#define M(n) METASIG_ATOM(ELEMENT_TYPE_MVAR) METASIG_ATOM(n)

#define GI(type, n, x) METASIG_ATOM(ELEMENT_TYPE_GENERICINST) type METASIG_ATOM(n) x

// The references to other types have special definition in some cases
#ifndef C
#define C(x) METASIG_ATOM(ELEMENT_TYPE_CLASS) METASIG_ATOM(CLASS__ ## x % 0x100) METASIG_ATOM(CLASS__ ## x / 0x100)
#endif
#ifndef g
#define g(x) METASIG_ATOM(ELEMENT_TYPE_VALUETYPE) METASIG_ATOM(CLASS__ ## x % 0x100) METASIG_ATOM(CLASS__ ## x / 0x100)
#endif
#ifndef Q
#define Q(x) METASIG_ATOM(ELEMENT_TYPE_CMOD_REQD) METASIG_ATOM(CLASS__ ## x % 0x100) METASIG_ATOM(CLASS__ ## x / 0x100)
#endif

#else

#define a(x) METASIG_ATOM(ELEMENT_TYPE_SZARRAY)
#define P(x) METASIG_ATOM(ELEMENT_TYPE_PTR)
#define r(x) METASIG_ATOM(ELEMENT_TYPE_BYREF)

#define G(n) METASIG_ATOM(ELEMENT_TYPE_VAR)
#define M(n) METASIG_ATOM(ELEMENT_TYPE_MVAR)

#define GI(type, n, x) METASIG_ATOM(ELEMENT_TYPE_GENERICINST)

// The references to other types have special definition in some cases
#ifndef C
#define C(x) METASIG_ATOM(ELEMENT_TYPE_CLASS)
#endif
#ifndef g
#define g(x) METASIG_ATOM(ELEMENT_TYPE_VALUETYPE)
#endif

#define Q(x)

#endif



// to avoid empty arguments for macros
#define _


// static methods:
DEFINE_METASIG_T(SM(Int_IntPtr_IntPtr_RetException, i I I, C(EXCEPTION)))
DEFINE_METASIG_T(SM(Type_CharPtr_RuntimeAssembly_Bool_Bool_IntPtr_RetRuntimeType, P(u) C(ASSEMBLY) F F I, C(CLASS)))
DEFINE_METASIG_T(SM(Type_RetIntPtr, C(TYPE), I))
DEFINE_METASIG(SM(RefIntPtr_IntPtr_IntPtr_Int_RetObj, r(I) I I i, j))
DEFINE_METASIG(SM(IntPtr_UInt_VoidPtr_RetObj, I K P(v), j))
DEFINE_METASIG(SM(Obj_IntPtr_RetIntPtr, j I, I))
DEFINE_METASIG(SM(VoidPtr_RetVoidPtr, P(v), P(v)))
DEFINE_METASIG(SM(Obj_VoidPtr_RetVoidPtr, j P(v), P(v)))
DEFINE_METASIG(SM(Obj_IntPtr_RetObj, j I, j))
DEFINE_METASIG(SM(Obj_RefIntPtr_RetVoid, j r(I), v))
DEFINE_METASIG(SM(Obj_RefBool_RetVoid, j r(F), v))
DEFINE_METASIG(SM(Obj_IntPtr_RetVoid, j I, v))
DEFINE_METASIG(SM(Obj_IntPtr_RetBool, j I, F))
DEFINE_METASIG(SM(Obj_IntPtr_IntPtr_Int_RetIntPtr, j I I i, I))
DEFINE_METASIG(SM(IntPtr_IntPtr_RefIntPtr_RetObj, I I r(I), j))
DEFINE_METASIG_T(SM(Obj_RefExInfo_RetVoid, j r(g(EXINFO)), v))
DEFINE_METASIG_T(SM(UInt_RefExInfo_RetVoid, K r(g(EXINFO)), v))
DEFINE_METASIG_T(SM(RefExInfo_UIntPtr_RetVoid, r(g(EXINFO)) U, v))
DEFINE_METASIG_T(SM(RefExInfo_RefExInfo_RetVoid, r(g(EXINFO)) r(g(EXINFO)), v))
#ifdef FEATURE_COMINTEROP
DEFINE_METASIG(SM(Obj_IntPtr_RefIntPtr_RefBool_RetIntPtr, j I r(I) r(F), I))
DEFINE_METASIG(SM(Obj_IntPtr_RefIntPtr_RetIntPtr, j I r(I), I))
#endif // FEATURE_COMINTEROP
#ifdef FEATURE_COMWRAPPERS
DEFINE_METASIG_T(SM(IntPtr_CreateObjectFlags_RetObj, I g(CREATEOBJECTFLAGS), j))
DEFINE_METASIG_T(SM(ManagedObjectWrapperHolder_RefGuid_RefIntPtr_RetInt, C(MANAGED_OBJECT_WRAPPER_HOLDER) r(g(GUID)) r(I), i))
#endif // FEATURE_COMWRAPPERS
#ifdef FEATURE_OBJCMARSHAL
DEFINE_METASIG_T(SM(Exception_Obj_RefIntPtr_RetVoidPtr, C(EXCEPTION) j r(I), P(v)))
#endif // FEATURE_OBJCMARSHAL
DEFINE_METASIG(SM(Int_RetVoid, i, v))
DEFINE_METASIG(SM(Int_RetObj, i, j))
DEFINE_METASIG(SM(Int_Int_RetVoid, i i, v))
DEFINE_METASIG(SM(Str_RetIntPtr, s, I))
DEFINE_METASIG(SM(Str_RetBool, s, F))
DEFINE_METASIG(SM(IntPtr_IntPtr_RetVoid, I I, v))
DEFINE_METASIG(SM(IntPtr_IntPtr_Int_Obj_RetIntPtr, I I i j, I))
DEFINE_METASIG(SM(IntPtr_IntPtr_IntPtr_RetVoid, I I I, v))
DEFINE_METASIG(SM(IntPtr_IntPtr_IntPtr_UShrt_IntPtr_RetVoid, I I I H I, v))
DEFINE_METASIG(SM(IntPtr_Int_IntPtr_RetObj, I i I, j))
DEFINE_METASIG(SM(IntPtr_IntPtr_Int_Bool_IntPtr_RetVoid, I I i F I, v))
DEFINE_METASIG(SM(IntPtr_IntPtr_Obj_RetVoid, I I j, v))
DEFINE_METASIG_T(SM(Obj_Array_RetVoid, j C(ARRAY), v))
DEFINE_METASIG(SM(Obj_IntPtr_Obj_RetVoid, j I j, v))
DEFINE_METASIG(SM(RetUIntPtr, _, U))
DEFINE_METASIG(SM(RetIntPtr, _, I))
DEFINE_METASIG(SM(RetUInt, _, K))
DEFINE_METASIG(SM(RetBool, _, F))
DEFINE_METASIG(SM(IntPtr_RetStr, I, s))
DEFINE_METASIG(SM(IntPtr_RetBool, I, F))
DEFINE_METASIG(SM(UInt_IntPtr_RetStr, K I, s))
DEFINE_METASIG_T(SM(RuntimeType_RuntimeMethodHandleInternal_RetMethodBase, C(CLASS) g(METHOD_HANDLE_INTERNAL), C(METHOD_BASE) ))
DEFINE_METASIG_T(SM(RuntimeType_IRuntimeFieldInfo_RetFieldInfo, C(CLASS) C(I_RT_FIELD_INFO), C(FIELD_INFO) ))
DEFINE_METASIG(SM(Char_Bool_Bool_RetByte, u F F, b))
DEFINE_METASIG(SM(Byte_RetChar, b, u))
DEFINE_METASIG(SM(Str_Bool_Bool_RefInt_RetIntPtr, s F F r(i), I))
DEFINE_METASIG(SM(IntPtr_Int_RetStr, I i, s))
DEFINE_METASIG_T(SM(Obj_PtrByte_RefCleanupWorkListElement_RetVoid, j P(b) r(C(CLEANUP_WORK_LIST_ELEMENT)), v))
DEFINE_METASIG_T(SM(SafeHandle_RefCleanupWorkListElement_RetIntPtr, C(SAFE_HANDLE) r(C(CLEANUP_WORK_LIST_ELEMENT)), I))
DEFINE_METASIG(SM(Obj_PtrByte_RetVoid, j P(b), v))
DEFINE_METASIG(SM(PtrByte_IntPtr_RetVoid, P(b) I, v))
DEFINE_METASIG(SM(Str_Bool_Bool_RefInt_RetArrByte, s F F r(i), a(b) ))
DEFINE_METASIG(SM(ArrByte_Int_PtrByte_Int_Int_RetVoid, a(b) i P(b) i i, v))
DEFINE_METASIG(SM(PtrByte_Int_ArrByte_Int_Int_RetVoid, P(b) i a(b) i i, v))
DEFINE_METASIG(SM(PtrByte_RetInt, P(b), i))
DEFINE_METASIG(SM(PtrSByt_RetInt, P(B), i))
DEFINE_METASIG(SM(IntPtr_RetIntPtr, I, I))
DEFINE_METASIG(SM(UIntPtr_RetIntPtr, U, I))
DEFINE_METASIG(SM(PtrByte_PtrByte_Int_RetVoid, P(b) P(b) i, v))
DEFINE_METASIG(SM(RefByte_RefByte_UIntPtr_RetVoid, r(b) r(b) U, v))
DEFINE_METASIG(SM(RefByte_Byte_UIntPtr_RetVoid, r(b) b U, v))
DEFINE_METASIG(SM(RefByte_UIntPtr_RetVoid, r(b) U, v))
DEFINE_METASIG(SM(PtrVoid_Byte_UInt_RetVoid, P(v) b K, v))
DEFINE_METASIG(SM(RefObj_IntPtr_RetVoid, r(j) I, v))
DEFINE_METASIG(SM(RefObj_RefIntPtr_RetVoid, r(j) r(I), v))
DEFINE_METASIG(SM(IntPtr_RefObj_IntPtr_RetVoid, I r(j) I, v))
DEFINE_METASIG(SM(IntPtr_RefObj_PtrIntPtr_RetVoid, I r(j) P(I), v))
DEFINE_METASIG(SM(IntPtr_RefObj_IntPtr_Int_RetVoid, I r(j) I i,v))
DEFINE_METASIG(SM(IntPtr_Int_IntPtr_Int_Int_Int_RetVoid, I i I i i i, v))
DEFINE_METASIG(SM(IntPtr_IntPtr_Int_Int_IntPtr_RetVoid, I I i i I, v))
DEFINE_METASIG(SM(IntPtr_RefObj_IntPtr_Obj_RetVoid, I r(j) I j, v))
DEFINE_METASIG(SM(Obj_Int_RetVoid, j i,v))
DEFINE_METASIG(SM(PtrVoid_Obj_RetObj, P(v) j, j))
DEFINE_METASIG(SM(PtrVoid_Obj_RetRefByte, P(v) j, r(b)))
DEFINE_METASIG_T(SM(ICustomMarshaler_RefObj_PtrIntPtr_RetVoid, C(ICUSTOM_MARSHALER) r(j) P(I), v))

DEFINE_METASIG(SM(RefDbl_Dbl_RetDbl, r(d) d, d))
DEFINE_METASIG(SM(RefDbl_Dbl_Dbl_RetDbl, r(d) d d, d))
DEFINE_METASIG(SM(Long_Long_RetLong, l l, l))
DEFINE_METASIG(SM(ULong_ULong_RetULong, L L, L))
DEFINE_METASIG(SM(RefLong_Long_RetLong, r(l) l, l))
DEFINE_METASIG(SM(RefLong_Long_Long_RetLong, r(l) l l, l))
DEFINE_METASIG(SM(RefFlt_Flt_RetFlt, r(f) f, f))
DEFINE_METASIG(SM(RefFlt_Flt_Flt_RetFlt, r(f) f f, f))
DEFINE_METASIG(SM(RefInt_Int_RetInt, r(i) i, i))
DEFINE_METASIG(SM(RefInt_Int_Int_RetInt, r(i) i i, i))
DEFINE_METASIG(SM(RefInt_Int_Int_RefBool_RetInt, r(i) i i r(F), i))
DEFINE_METASIG(SM(RefIntPtr_IntPtr_RetIntPtr, r(I) I, I))
DEFINE_METASIG(SM(RefIntPtr_IntPtr_IntPtr_RetIntPtr, r(I) I I, I))
DEFINE_METASIG(SM(RefObj_Obj_RetObj, r(j) j, j))
DEFINE_METASIG(SM(RefObj_Obj_Obj_RetObj, r(j) j j, j))
DEFINE_METASIG(SM(ObjIntPtr_RetVoid, j I, v))

DEFINE_METASIG(SM(RefBool_RetBool, r(F), F))
DEFINE_METASIG(SM(RefBool_Bool, r(F) F, v))
DEFINE_METASIG(SM(RefSByt_RetSByt, r(B), B))
DEFINE_METASIG(SM(RefSByt_SByt, r(B) B, v))
DEFINE_METASIG(SM(RefByte_RetByte, r(b), b))
DEFINE_METASIG(SM(RefByte_Byte, r(b) b, v))
DEFINE_METASIG(SM(RefByte_RefByte_UInt_RetVoid, r(b) r(b) K, v))
DEFINE_METASIG(SM(RefByte_Byte_UInt_RetVoid, r(b) b K, v))
DEFINE_METASIG(SM(RefShrt_RetShrt, r(h), h))
DEFINE_METASIG(SM(RefShrt_Shrt, r(h) h, v))
DEFINE_METASIG(SM(RefUShrt_RetUShrt, r(H), H))
DEFINE_METASIG(SM(RefUShrt_UShrt, r(H) H, v))
DEFINE_METASIG(SM(RefInt_RetInt, r(i), i))
DEFINE_METASIG(SM(RefInt_Int, r(i) i, v))
DEFINE_METASIG(SM(RefUInt_RetUInt, r(K), K))
DEFINE_METASIG(SM(RefUInt_UInt, r(K) K, v))
DEFINE_METASIG(SM(RefLong_RetLong, r(l), l))
DEFINE_METASIG(SM(RefLong_Long, r(l) l, v))
DEFINE_METASIG(SM(RefULong_RetULong, r(L), L))
DEFINE_METASIG(SM(RefULong_ULong, r(L) L, v))
DEFINE_METASIG(SM(RefIntPtr_RetIntPtr, r(I), I))
DEFINE_METASIG(SM(RefIntPtr_IntPtr, r(I) I, v))
DEFINE_METASIG(SM(RefUIntPtr_RetUIntPtr, r(U), U))
DEFINE_METASIG(SM(RefUIntPtr_UIntPtr, r(U) U, v))
DEFINE_METASIG(SM(RefFlt_RetFlt, r(f), f))
DEFINE_METASIG(SM(RefFlt_Flt, r(f) f, v))
DEFINE_METASIG(SM(RefDbl_RetDbl, r(d), d))
DEFINE_METASIG(SM(RefDbl_Dbl, r(d) d, v))
DEFINE_METASIG(GM(RefT_RetT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, r(M(0)), M(0)))
DEFINE_METASIG(GM(RefT_T, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, r(M(0)) M(0), v))

DEFINE_METASIG(GM(RefByte_RetT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, r(b), M(0)))
DEFINE_METASIG(GM(RefByte_T_RetVoid, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, r(b) M(0), v))
DEFINE_METASIG(GM(PtrVoid_RetT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, P(v), M(0)))
DEFINE_METASIG(GM(PtrVoid_T_RetVoid, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, P(v) M(0), v))

DEFINE_METASIG(GM(RefT_RetRefT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, r(M(0)), r(M(0))))
DEFINE_METASIG(GM(VoidPtr_RetRefT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, P(v), r(M(0))))
DEFINE_METASIG(GM(RefTFrom_RetRefTTo, IMAGE_CEE_CS_CALLCONV_DEFAULT, 2, r(M(0)), r(M(1))))
DEFINE_METASIG(GM(Obj_RetT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, j, M(0)))
DEFINE_METASIG(GM(RefT_Int_RetRefT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, r(M(0)) i, r(M(0))))
DEFINE_METASIG(GM(RefT_IntPtr_RetRefT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, r(M(0)) I, r(M(0))))
DEFINE_METASIG(GM(RefT_UIntPtr_RetRefT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, r(M(0)) U, r(M(0))))
DEFINE_METASIG(GM(PtrVoid_Int_RetPtrVoid, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, P(v) i, P(v)))
DEFINE_METASIG(GM(RefT_RetVoid, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, r(M(0)), v))
DEFINE_METASIG(GM(PtrVoid_RefT_RetVoid, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, P(v) r(M(0)), v))
DEFINE_METASIG(GM(RefT_PtrVoid_RetVoid, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, r(M(0)) P(v), v))

DEFINE_METASIG(GM(ArrT_RetRefT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, a(M(0)), r(M(0))))
DEFINE_METASIG_T(SM(Array_RetRefByte, C(ARRAY), r(b)))

DEFINE_METASIG_T(SM(SafeHandle_RefBool_RetIntPtr, C(SAFE_HANDLE) r(F), I ))
DEFINE_METASIG_T(SM(SafeHandle_RetVoid, C(SAFE_HANDLE), v ))

DEFINE_METASIG_T(SM(RetMethodBase, _, C(METHOD_BASE)))
DEFINE_METASIG(SM(RetVoid, _, v))
DEFINE_METASIG(SM(Str_IntPtr_Int_RetVoid, s I i, v))
DEFINE_METASIG(SM(Int_RetIntPtr, i, I))
DEFINE_METASIG(SM(Int_IntPtr_RetIntPtr, i I, I))

DEFINE_METASIG_T(SM(DateTime_RetDbl, g(DATE_TIME), d))
DEFINE_METASIG(SM(Dbl_RetLong, d, l))

DEFINE_METASIG(SM(IntPtr_RetObj, I, j))
DEFINE_METASIG_T(SM(Int_RetException, i, C(EXCEPTION)))
DEFINE_METASIG_T(SM(RetException, _, C(EXCEPTION)))
DEFINE_METASIG(SM(Int_IntPtr_RetObj, i I, j))
DEFINE_METASIG(SM(IntPtr_IntPtr_Int_RetVoid, I I i, v))
DEFINE_METASIG_T(SM(Exception_RetInt, C(EXCEPTION), i))


DEFINE_METASIG(SM(IntPtr_RetVoid, I, v))
DEFINE_METASIG(SM(IntPtr_Bool_RetVoid, I F, v))
DEFINE_METASIG(SM(IntPtr_UInt_IntPtr_RetVoid, I K I, v))
DEFINE_METASIG(SM(IntPtr_RetUInt, I, K))
DEFINE_METASIG(SM(PtrChar_RetInt, P(u), i))
DEFINE_METASIG(SM(PtrChar_RetVoid, P(u), v))
DEFINE_METASIG(SM(IntPtr_IntPtr_RetIntPtr, I I, I))
DEFINE_METASIG(SM(IntPtr_IntPtr_Int_RetIntPtr, I I i, I))
DEFINE_METASIG(SM(PtrVoid_RetVoid, P(v), v))
DEFINE_METASIG(SM(PtrVoid_PtrVoid_RetVoid, P(v) P(v), v))
DEFINE_METASIG(SM(PtrVoid_PtrVoid_PtrVoid_RetVoid, P(v) P(v) P(v), v))
DEFINE_METASIG(SM(PtrVoid_PtrVoid_UInt_RetVoid, P(v) P(v) K, v))
DEFINE_METASIG(IM(Obj_RetBool, j, F))
DEFINE_METASIG(SM(Obj_RetVoid, j, v))
DEFINE_METASIG(SM(Obj_RetInt, j, i))
DEFINE_METASIG(SM(Obj_RetIntPtr, j, I))
DEFINE_METASIG(SM(Obj_RetObj, j, j))
DEFINE_METASIG(SM(Obj_RetArrByte, j, a(b)))
DEFINE_METASIG(SM(Obj_Bool_RetArrByte, j F, a(b)))
DEFINE_METASIG(SM(Obj_Obj_RefArrByte_RetArrByte, j j r(a(b)), a(b)))

#ifdef FEATURE_COMINTEROP
DEFINE_METASIG_T(SM(Obj_Int_RefComVariant_RetVoid, j i r(g(COMVARIANT)), v))
DEFINE_METASIG_T(SM(Obj_RefComVariant_RetVoid, j r(g(COMVARIANT)), v))
DEFINE_METASIG_T(SM(RefComVariant_RetObject, r(g(COMVARIANT)), j))
DEFINE_METASIG_T(IM(RuntimeTypeHandle_RefBool_RefIntPtr_RetVoid, g(RT_TYPE_HANDLE) r(F) r(I), v))

#endif


DEFINE_METASIG(SM(Str_RetInt, s, i))
DEFINE_METASIG_T(SM(Str_RetICustomMarshaler, s, C(ICUSTOM_MARSHALER)))
DEFINE_METASIG(SM(Int_Str_RetIntPtr, i s, I))
DEFINE_METASIG(SM(Int_Str_IntPtr_RetIntPtr, i s I, I))
DEFINE_METASIG(SM(Int_Str_IntPtr_Int_RetVoid, i s I i, v))
DEFINE_METASIG(SM(Str_IntPtr_RetIntPtr, s I, I))
DEFINE_METASIG(SM(Str_Bool_Int_RetV, s F i, v))

DEFINE_METASIG_T(SM(Type_RetObj, C(TYPE), j))
DEFINE_METASIG_T(SM(Type_RetInt, C(TYPE), i))

DEFINE_METASIG(IM(Long_RetVoid, l, v))
DEFINE_METASIG(IM(IntPtr_Int_RetVoid, I i, v))
DEFINE_METASIG(IM(IntInt_RetArrByte, i i, a(b)))
DEFINE_METASIG(IM(RetIntPtr, _, I))
DEFINE_METASIG(IM(RetInt, _, i))
DEFINE_METASIG_T(IM(RetModule, _, C(MODULE)))
DEFINE_METASIG_T(IM(PtrNativeAssemblyNameParts, P(g(NATIVE_ASSEMBLY_NAME_PARTS)), v))
DEFINE_METASIG(SM(PtrCharPtrVoid, P(u) P(v), v))
DEFINE_METASIG(IM(RetObj, _, j))
DEFINE_METASIG(SM(RetObj, _, j))
DEFINE_METASIG_T(IM(RetIEnumerator, _, C(IENUMERATOR)))
DEFINE_METASIG(IM(RetStr, _, s))
DEFINE_METASIG(IM(RetLong, _, l))

DEFINE_METASIG_T(IM(RetType, _, C(TYPE)))
DEFINE_METASIG(IM(RetVoid, _, v))
DEFINE_METASIG(IM(RetBool, _, F))
DEFINE_METASIG(IM(RetArrByte, _, a(b)))
DEFINE_METASIG_T(IM(RetArrParameterInfo, _, a(C(PARAMETER))))
DEFINE_METASIG_T(IM(RetCultureInfo, _, C(CULTURE_INFO)))

DEFINE_METASIG(IM(Bool_RetIntPtr, F, I))
DEFINE_METASIG_T(IM(Bool_RetMethodInfo, F, C(METHOD_INFO)))
DEFINE_METASIG(SM(Bool_RetStr, F, s))
DEFINE_METASIG(IM(Bool_Bool_RetStr, F F, s))

DEFINE_METASIG(IM(PtrChar_RetVoid, P(u), v))
DEFINE_METASIG(IM(PtrChar_Int_Int_RetVoid, P(u) i i, v))
DEFINE_METASIG_T(IM(ReadOnlySpanOfByte_RetVoid, GI(g(READONLY_SPAN), 1, b), v))
DEFINE_METASIG_T(IM(ReadOnlySpanOfChar_RetVoid, GI(g(READONLY_SPAN), 1, u), v))
DEFINE_METASIG(IM(PtrSByt_RetVoid, P(B), v))
DEFINE_METASIG(IM(PtrSByt_Int_Int_RetVoid, P(B) i i, v))
DEFINE_METASIG_T(IM(PtrSByt_Int_Int_Encoding_RetVoid, P(B) i i C(ENCODING), v))
DEFINE_METASIG(IM(PtrChar_Int_RetVoid, P(u) i, v))
DEFINE_METASIG(IM(PtrSByt_Int_RetVoid, P(B) i, v))

DEFINE_METASIG(SM(ArrChar_RetStr, a(u), s))
DEFINE_METASIG(SM(ArrChar_Int_Int_RetStr, a(u) i i, s))
DEFINE_METASIG(SM(Char_Int_RetStr, u i, s))
DEFINE_METASIG(SM(PtrChar_RetStr, P(u), s))
DEFINE_METASIG(SM(PtrChar_Int_Int_RetStr, P(u) i i, s))
DEFINE_METASIG_T(SM(ReadOnlySpanOfChar_RetStr, GI(g(READONLY_SPAN), 1, u), s))
DEFINE_METASIG(SM(PtrSByt_RetStr, P(B), s))
DEFINE_METASIG(SM(PtrSByt_Int_Int_RetStr, P(B) i i, s))
DEFINE_METASIG_T(SM(PtrSByt_Int_Int_Encoding_RetStr, P(B) i i C(ENCODING), s))
DEFINE_METASIG(IM(Obj_Int_RetIntPtr, j i, I))

DEFINE_METASIG(IM(ArrByte_Int_Int_RetVoid, a(b) i i, v))

DEFINE_METASIG(IM(Char_Char_RetStr, u u, s))
DEFINE_METASIG(IM(Char_Int_RetVoid, u i, v))
DEFINE_METASIG_T(SM(RetCultureInfo, _, C(CULTURE_INFO)))
DEFINE_METASIG_T(SM(CultureInfo_RetVoid, C(CULTURE_INFO), v))
DEFINE_METASIG(IM(Dbl_RetVoid, d, v))
DEFINE_METASIG(IM(Flt_RetVoid, f, v))
DEFINE_METASIG(IM(Int_RetInt, i, i))
DEFINE_METASIG(IM(Int_RefIntPtr_RefIntPtr_RefIntPtr_RetVoid, i r(I) r(I) r(I), v))
DEFINE_METASIG(IM(Int_RetStr, i, s))
DEFINE_METASIG(IM(Int_RetVoid, i, v))
DEFINE_METASIG(IM(Int_RetBool, i, F))
DEFINE_METASIG(IM(Int_Int_RetVoid, i i, v))
DEFINE_METASIG(IM(Int_Int_Int_RetVoid, i i i, v))
DEFINE_METASIG(IM(Int_Int_Int_Int_RetVoid, i i i i, v))

DEFINE_METASIG_T(IM(Exception_RetVoid, C(EXCEPTION), v))

DEFINE_METASIG(IM(IntPtr_RetVoid, I, v))
DEFINE_METASIG_T(IM(RefGuid_RetIntPtr, r(g(GUID)), I))

DEFINE_METASIG(IM(Obj_RetInt, j, i))
DEFINE_METASIG(IM(Obj_RetVoid, j, v))
DEFINE_METASIG(IM(Obj_RetObj, j, j))
DEFINE_METASIG(IM(Obj_IntPtr_RetVoid, j I, v))
DEFINE_METASIG(IM(Obj_UIntPtr_RetVoid, j U, v))
DEFINE_METASIG(IM(Obj_IntPtr_IntPtr_RetVoid, j I I, v))
DEFINE_METASIG(IM(Obj_IntPtr_IntPtr_IntPtr_RetVoid, j I I I, v))
DEFINE_METASIG(IM(Obj_IntPtr_IntPtr_IntPtr_IntPtr_RetVoid, j I I I I, v))
DEFINE_METASIG(IM(IntPtr_UInt_IntPtr_IntPtr_RetVoid, I K I I, v))
DEFINE_METASIG(IM(Obj_Bool_RetVoid, j F, v))
#ifdef FEATURE_COMINTEROP
DEFINE_METASIG(SM(Obj_RetStr, j, s))
DEFINE_METASIG_T(IM(Str_BindingFlags_Obj_ArrObj_ArrBool_ArrInt_ArrType_Type_RetObj, s g(BINDING_FLAGS) j a(j) a(F) a(i) a(C(TYPE)) C(TYPE), j))
#endif // FEATURE_COMINTEROP
DEFINE_METASIG_T(IM(Obj_Obj_BindingFlags_Binder_CultureInfo_RetVoid, j j g(BINDING_FLAGS) C(BINDER) C(CULTURE_INFO), v))
DEFINE_METASIG_T(IM(Obj_Obj_BindingFlags_Binder_ArrObj_CultureInfo_RetVoid, j j g(BINDING_FLAGS) C(BINDER) a(j) C(CULTURE_INFO), v))
DEFINE_METASIG_T(IM(Obj_BindingFlags_Binder_ArrObj_CultureInfo_RetObj, j g(BINDING_FLAGS) C(BINDER) a(j) C(CULTURE_INFO), j))
DEFINE_METASIG(IM(RefObject_RetBool, r(j), F))
DEFINE_METASIG_T(IM(Class_RetObj, C(CLASS), j))
DEFINE_METASIG(IM(Int_VoidPtr_RetVoid, i P(v), v))
DEFINE_METASIG(IM(VoidPtr_RetVoid, P(v), v))
DEFINE_METASIG(SM(VoidPtr_RetObj, P(v), j))

DEFINE_METASIG_T(IM(Str_RetModule, s, C(MODULE)))
DEFINE_METASIG_T(SM(Assembly_Str_RetAssembly, C(ASSEMBLY) s, C(ASSEMBLY)))
DEFINE_METASIG_T(SM(Str_Bool_RetAssembly, s F, C(ASSEMBLY)))
DEFINE_METASIG(IM(Str_Str_Obj_RetVoid, s s j, v))
DEFINE_METASIG(IM(Str_Str_Str_Obj_RetVoid, s s s j, v))
DEFINE_METASIG(IM(Str_Str_Str_Obj_Bool_RetVoid, s s s j F, v))
DEFINE_METASIG(IM(Str_Str_RefObj_RetVoid, s s r(j), v))
DEFINE_METASIG(SM(Str_RetStr, s, s))
DEFINE_METASIG_T(SM(Str_CultureInfo_RetStr, s C(CULTURE_INFO), s))
DEFINE_METASIG_T(SM(Str_CultureInfo_RefBool_RetStr, s C(CULTURE_INFO) r(F), s))
DEFINE_METASIG(SM(PtrPtrChar_PtrPtrChar_Int_RetVoid, P(P(u)) P(P(u)) i, v))
DEFINE_METASIG(SM(PtrChar_Int_PtrPtrChar_RetArrStr, P(u) i P(P(u)), a(s)))
DEFINE_METASIG(SM(RefBool_RefBool_RetVoid, r(F) r(F), v))
DEFINE_METASIG_T(IM(Str_Exception_RetVoid, s C(EXCEPTION), v))
DEFINE_METASIG(IM(Str_Obj_RetVoid, s j, v))
DEFINE_METASIG(IM(Str_Str_RetStr, s s, s))
DEFINE_METASIG(IM(Str_Str_RetVoid, s s, v))
DEFINE_METASIG(IM(Str_Str_Str_RetVoid, s s s, v))
DEFINE_METASIG(IM(Str_Int_RetVoid, s i, v))
DEFINE_METASIG(IM(Str_Str_Int_RetVoid, s s i, v))
DEFINE_METASIG(IM(Str_Str_Str_Int_RetVoid, s s s i, v))
DEFINE_METASIG_T(IM(Str_BindingFlags_Binder_Obj_ArrObj_ArrParameterModifier_CultureInfo_ArrStr_RetObj, \
                 s g(BINDING_FLAGS) C(BINDER) j a(j) a(g(PARAMETER_MODIFIER)) C(CULTURE_INFO) a(s), j))
DEFINE_METASIG_T(SM(Delegate_RetIntPtr, C(DELEGATE), I))
DEFINE_METASIG_T(SM(RuntimeTypeHandle_RetType, g(RT_TYPE_HANDLE), C(TYPE)))
DEFINE_METASIG_T(SM(RuntimeTypeHandle_RetIntPtr, g(RT_TYPE_HANDLE), I))
DEFINE_METASIG_T(SM(RuntimeMethodHandle_RetIntPtr, g(METHOD_HANDLE), I))
DEFINE_METASIG_T(SM(IntPtr_Type_RetDelegate, I C(TYPE), C(DELEGATE)))
DEFINE_METASIG_T(SM(IntPtr_RuntimeType_RetDelegate, I C(CLASS), C(DELEGATE)))

DEFINE_METASIG(IM(RetRefByte, _, r(b)))
DEFINE_METASIG_T(IM(Type_RetArrObj, C(TYPE) F, a(j)))
DEFINE_METASIG(IM(Bool_RetVoid, F, v))
DEFINE_METASIG_T(IM(BindingFlags_RetArrFieldInfo, g(BINDING_FLAGS), a(C(FIELD_INFO))))
DEFINE_METASIG_T(IM(BindingFlags_RetArrMethodInfo, g(BINDING_FLAGS), a(C(METHOD_INFO))))
DEFINE_METASIG_T(IM(BindingFlags_RetArrPropertyInfo, g(BINDING_FLAGS), a(C(PROPERTY_INFO))))
DEFINE_METASIG(IM(ArrByte_RetVoid, a(b), v))
DEFINE_METASIG(IM(ArrChar_RetVoid, a(u), v))
DEFINE_METASIG(IM(ArrChar_Int_Int_RetVoid, a(u) i i, v))
DEFINE_METASIG_T(IM(ArrType_ArrException_Str_RetVoid, a(C(TYPE)) a(C(EXCEPTION)) s, v))
DEFINE_METASIG(IM(RefInt_RefInt_RefInt_RetArrByte, r(i) r(i) r(i), a(b)))
DEFINE_METASIG_T(IM(RefInt_RetRuntimeType, r(i) , C(CLASS)))
DEFINE_METASIG_T(SM(IntPtr_RetRuntimeType, I , C(CLASS)))
DEFINE_METASIG_T(IM(RuntimeType_RetVoid, C(CLASS) , v))

DEFINE_METASIG_T(IM(RuntimeArgumentHandle_PtrVoid_RetVoid, g(ARGUMENT_HANDLE) P(v), v))

DEFINE_METASIG_T(SM(Assembly_RetVoid, C(ASSEMBLY), v))
DEFINE_METASIG_T(SM(Assembly_Str_RetArrAssembly, C(ASSEMBLY) s, a(C(ASSEMBLY))))
DEFINE_METASIG(SM(Str_RetArrStr, s, a(s)))

// Execution Context
DEFINE_METASIG_T(SM(SyncCtx_ArrIntPtr_Bool_Int_RetInt, C(SYNCHRONIZATION_CONTEXT) a(I) F i, i))

// Exception
DEFINE_METASIG(IM(RefUInt_RetStr, r(K), s))

#ifdef FEATURE_COMINTEROP
// The signature of the method System.Runtime.InteropServices.ICustomQueryInterface.GetInterface
DEFINE_METASIG_T(IM(RefGuid_OutIntPtr_RetCustomQueryInterfaceResult, r(g(GUID)) r(I), g(CUSTOMQUERYINTERFACERESULT)))
#endif //FEATURE_COMINTEROP

// Assembly Load Context
DEFINE_METASIG_T(SM(RefGuid_RefGuid_RetVoid, r(g(GUID)) r(g(GUID)) , v))
DEFINE_METASIG_T(SM(RefGuid_RetVoid, r(g(GUID)), v))

DEFINE_METASIG_T(SM(IntPtr_AssemblyName_RetAssembly, I C(ASSEMBLY_NAME), C(ASSEMBLY)))
DEFINE_METASIG_T(SM(Str_AssemblyBase_IntPtr_RetIntPtr, s C(ASSEMBLYBASE) I, I))
DEFINE_METASIG_T(SM(Str_AssemblyBase_Bool_UInt_RetIntPtr, s C(ASSEMBLYBASE) F K, I))

// For FailFast
DEFINE_METASIG(SM(Str_RetVoid, s, v))
DEFINE_METASIG_T(SM(Str_Exception_RetVoid, s C(EXCEPTION), v))
DEFINE_METASIG_T(SM(Str_Exception_Str_RetVoid, s C(EXCEPTION) s, v))

// fields - e.g.:
// DEFINE_METASIG(Fld(PtrVoid, P(v)))

// Runtime Helpers
DEFINE_METASIG(SM(Obj_Obj_Bool_RetVoid, j j F, v))

DEFINE_METASIG_T(IM(Dec_RetVoid, g(DECIMAL), v))
DEFINE_METASIG_T(IM(Currency_RetVoid, g(CURRENCY), v))
DEFINE_METASIG_T(SM(RefDec_RetVoid, r(g(DECIMAL)), v))

DEFINE_METASIG_T(IM(Exception_RetTaskOfT, C(EXCEPTION), GI(C(TASK_1), 1, G(0))))
DEFINE_METASIG_T(IM(T_RetTaskOfT, G(0), GI(C(TASK_1), 1, G(0))))

DEFINE_METASIG_T(IM(Exception_RetTask, C(EXCEPTION), C(TASK)))
DEFINE_METASIG_T(IM(RetTask, _, C(TASK)))

DEFINE_METASIG_T(IM(Exception_RetValueTaskOfT, C(EXCEPTION), GI(g(VALUETASK_1), 1, G(0))))
DEFINE_METASIG_T(IM(T_RetValueTaskOfT, G(0), GI(g(VALUETASK_1), 1, G(0))))

DEFINE_METASIG_T(IM(Exception_RetValueTask, C(EXCEPTION), g(VALUETASK)))
DEFINE_METASIG_T(IM(RetValueTask, _, g(VALUETASK)))

DEFINE_METASIG_T(GM(Exception_RetTaskOfT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, C(EXCEPTION), GI(C(TASK_1), 1, M(0))))
DEFINE_METASIG_T(GM(T_RetTaskOfT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, M(0), GI(C(TASK_1), 1, M(0))))
DEFINE_METASIG_T(GM(Exception_RetValueTaskOfT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, C(EXCEPTION), GI(g(VALUETASK_1), 1, M(0))))
DEFINE_METASIG_T(GM(T_RetValueTaskOfT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, M(0), GI(g(VALUETASK_1), 1, M(0))))

DEFINE_METASIG_T(SM(RetTask, , C(TASK)))
DEFINE_METASIG_T(SM(RetValueTask, , g(VALUETASK)))
DEFINE_METASIG_T(SM(Exception_RetTask, C(EXCEPTION), C(TASK)))
DEFINE_METASIG_T(SM(Exception_RetValueTask, C(EXCEPTION), g(VALUETASK)))

DEFINE_METASIG(GM(RefT_T_T_RetT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, r(M(0)) M(0) M(0), M(0)))
DEFINE_METASIG(SM(RefObject_Object_Object_RetObject, r(j) j j, j))
DEFINE_METASIG(SM(RefByte_Byte_Byte_RetByte, r(b) b b, b))
DEFINE_METASIG(SM(RefUShrt_UShrt_UShrt_RetUShrt, r(H) H H, H))

DEFINE_METASIG_T(SM(RefCleanupWorkListElement_RetVoid, r(C(CLEANUP_WORK_LIST_ELEMENT)), v))
DEFINE_METASIG_T(SM(RefCleanupWorkListElement_SafeHandle_RetIntPtr, r(C(CLEANUP_WORK_LIST_ELEMENT)) C(SAFE_HANDLE), I))
DEFINE_METASIG_T(SM(RefCleanupWorkListElement_Obj_RetVoid, r(C(CLEANUP_WORK_LIST_ELEMENT)) j, v))

DEFINE_METASIG(SM(PtrVoid_RetPtrVoid, P(v), P(v)))
DEFINE_METASIG(IM(PtrVoid_RetVoid, P(v), v))

DEFINE_METASIG_T(SM(IDynamicInterfaceCastable_RuntimeType_Bool_RetBool, C(IDYNAMICINTERFACECASTABLE) C(CLASS) F, F))
DEFINE_METASIG_T(SM(IDynamicInterfaceCastable_RuntimeType_RetRtType, C(IDYNAMICINTERFACECASTABLE) C(CLASS), C(CLASS)))

DEFINE_METASIG_T(IM(ArrByte_Int_Int_AsyncCallback_Object_RetIAsyncResult, a(b) i i C(ASYNCCALLBACK) j, C(IASYNCRESULT)))
DEFINE_METASIG_T(IM(IAsyncResult_RetInt, C(IASYNCRESULT), i))
DEFINE_METASIG_T(IM(IAsyncResult_RetVoid, C(IASYNCRESULT), v))

DEFINE_METASIG(IM(Int_RetRefT, i, r(G(0))))
DEFINE_METASIG_T(IM(Int_RetReadOnlyRefT, i, Q(INATTRIBUTE) r(G(0))))

DEFINE_METASIG(GM(RetT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, _, M(0)))

DEFINE_METASIG_T(SM(Array_Int_Array_Int_Int_RetVoid, C(ARRAY) i C(ARRAY) i i, v))
DEFINE_METASIG_T(SM(Array_Int_Obj_RetVoid, C(ARRAY) i j, v))
DEFINE_METASIG_T(SM(Array_Int_PtrVoid_RetRefObj, C(ARRAY) i P(v), r(j)))
DEFINE_METASIG(SM(ArrObject_IntPtr_Obj_RetVoid, a(j) I j, v))
DEFINE_METASIG(SM(ArrObject_IntPtr_PtrVoid_RetRefObj, a(j) I P(v), r(j)))

DEFINE_METASIG(SM(Obj_IntPtr_Bool_RetVoid, j I F, v))
DEFINE_METASIG(SM(IntPtr_Obj_RetVoid, I j, v))
DEFINE_METASIG_T(SM(IntPtr_Type_RetVoid, I C(TYPE), v))

DEFINE_METASIG_T(IM(Str_SpanOfByte_RetVoid, s GI(g(SPAN), 1, b), v))
DEFINE_METASIG(IM(RetPtrByte, , P(b)))
DEFINE_METASIG(IM(VoidPtr_Int_RetVoid, P(v) i, v))

DEFINE_METASIG(SM(PtrByte_RetStr, P(b), s))
DEFINE_METASIG(SM(Str_RetPtrByte, s, P(b)))
DEFINE_METASIG(SM(PtrByte_RetVoid, P(b), v))

DEFINE_METASIG_T(SM(RetContinuation, , C(CONTINUATION)))
DEFINE_METASIG(GM(T_RetVoid, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, M(0), v))
DEFINE_METASIG_T(SM(Continuation_RetTask, C(CONTINUATION), C(TASK)))
DEFINE_METASIG_T(GM(Continuation_RetTaskOfT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, C(CONTINUATION), GI(C(TASK_1), 1, M(0))))
DEFINE_METASIG_T(SM(Continuation_RetValueTask, C(CONTINUATION), g(VALUETASK)))
DEFINE_METASIG_T(GM(Continuation_RetValueTaskOfT, IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, C(CONTINUATION), GI(g(VALUETASK_1), 1, M(0))))

// Undefine macros in case we include the file again in the compilation unit

#undef  DEFINE_METASIG
#undef  DEFINE_METASIG_T

#undef METASIG_BODY
#undef METASIG_ATOM
#undef METASIG_RECURSE


#undef SM
#undef IM
#undef GM
#undef Fld

#undef a
#undef P
#undef r
#undef b
#undef u
#undef d
#undef f
#undef i
#undef K
#undef I
#undef U
#undef l
#undef L
#undef h
#undef H
#undef v
#undef B
#undef F
#undef j
#undef s
#undef C
#undef g
#undef T
#undef G
#undef M
#undef GI
#undef Q

#undef _


#endif // DEFINE_METASIG
