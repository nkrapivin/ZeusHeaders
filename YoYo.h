#pragma once
#ifndef _YOYO_H_
#define _YOYO_H_ 1
#include <exception>
#include <string>
#include "stdafx.h"

#define MASK_KIND_RVALUE (0x0ffffff)

class YYObjectBase;
class CScriptRef;
struct RValue;
class CWeakRef;
class RefDynamicArrayOfRValue;

template<typename T> class YYReference {
	T   m_refThing;
	int m_refCount;
	int m_size;

public:
	void inc() { ++m_refCount; }
	T    get()  const { return m_refThing; }
	int  size() const { return m_size; }
};

enum RVALUE_KIND {
	K_REAL,
	K_STRING,
	K_ARRAY,
	K_PTR,
	K_VEC3,
	K_UNDEFINED,
	K_OBJECT,
	K_INT32,
	K_VEC4,
	K_VEC44,
	K_INT64,
	K_ACCESSOR,
	K_NULL,
	K_BOOL,
	K_ITERATOR,
	K_REF,
	K_UNSET = MASK_KIND_RVALUE
};

enum EJSRetValBool {
	EJSRVB_FALSE = 0,
	EJSRVB_TRUE = 1,
	EJSRVB_TYPE_ERROR = 2
};

enum EHasInstanceRetVal {
	E_FALSE = 0,
	E_TRUE = 1,
	E_TYPE_ERROR = 2
};

enum OBJECT_KIND {
	OBJECT_KIND_YYOBJECTBASE = 0,
	OBJECT_KIND_CINSTANCE,
	OBJECT_KIND_ACCESSOR,
	OBJECT_KIND_SCRIPTREF,
	OBJECT_KIND_PROPERTY,
	OBJECT_KIND_ARRAY,
	OBJECT_KIND_WEAKREF,

	OBJECT_KIND_CONTAINER,

	OBJECT_KIND_SEQUENCE,
	OBJECT_KIND_SEQUENCEINSTANCE,
	OBJECT_KIND_SEQUENCETRACK,
	OBJECT_KIND_SEQUENCECURVE,
	OBJECT_KIND_SEQUENCECURVECHANNEL,
	OBJECT_KIND_SEQUENCECURVEPOINT,
	OBJECT_KIND_SEQUENCEKEYFRAMESTORE,
	OBJECT_KIND_SEQUENCEKEYFRAME,
	OBJECT_KIND_SEQUENCEKEYFRAMEDATA,
	OBJECT_KIND_SEQUENCEEVALTREE,
	OBJECT_KIND_SEQUENCEEVALNODE,
	OBJECT_KIND_SEQUENCEEVENT,

	OBJECT_KIND_NINESLICE,

	OBJECT_KIND_MAX
};

#pragma pack(push, 4)
union RUnion {
	int i32;
	long long i64;
	double f64;
	void* ptr;
	YYObjectBase* obj;
	CScriptRef* csp;
	YYReference<const char*>* str;
	RefDynamicArrayOfRValue* arr;
};

struct RValue {
	RUnion v;
	int flags;
	int kind;

	RValue() {
		kind = K_UNDEFINED;
		flags = 0;
		v.i64 = 0ll;
	}

	RValue(bool a) : RValue() {
		kind = K_BOOL;
		v.f64 = a ? 1.0 : 0.0;
	}

	RValue(int a) : RValue() {
		kind = K_INT32;
		v.i32 = a;
	}

	RValue(long long a) : RValue() {
		kind = K_INT64;
		v.i64 = a;
	}

	RValue(double a) : RValue() {
		kind = K_REAL;
		v.f64 = a;
	}

	int getFlags() const { return flags; }
	int  getType() const { return kind & MASK_KIND_RVALUE; }

	double toDouble() const {
		switch (getType()) {
		case K_REAL: return v.f64;
		case K_INT32: return v.i32;
		case K_INT64: return static_cast<double>(v.i64);
		case K_BOOL: return v.f64;
		}

		throw std::exception();
	}

	int toInt32() const {
		switch (getType()) {
		case K_REAL: return static_cast<int>(v.f64);
		case K_INT32: return v.i32;
		case K_INT64: return static_cast<int>(v.i64);
		case K_BOOL: return (v.f64 > 0.5) ? 1 : 0;
		}

		throw std::exception();
	}

	long long toInt64() const {
		switch (getType()) {
		case K_REAL: return static_cast<long long>(v.f64);
		case K_INT32: return v.i32;
		case K_INT64: return v.i64;
		case K_BOOL: return (v.f64 > 0.5) ? 1ll : 0ll;
		}

		throw std::exception();
	}

	bool toBoolean() const {
		switch (getType()) {
		case K_REAL: return v.f64 > 0.5;
		case K_INT32: return v.i32 > 0;
		case K_INT64: return v.i64 > 0;
		case K_BOOL: return v.f64 > 0.5;
		}

		throw std::exception();
	}

	std::string toString() const {
		switch (getType()) {
		case K_REAL: return std::to_string(v.f64);
		case K_INT32: return std::to_string(v.i32);
		case K_INT64: return std::to_string(v.i64);
		case K_BOOL: return (v.f64 > 0.5) ? "true" : "false";
		case K_STRING: return v.str->get();
		}

		throw std::exception();
	}
};
#pragma pack(pop)

class GCContext {
	RValue* pRValueFreeList;
	RValue* pRValueFreeListTail;
	RValue** pRValsToDecRef;
	bool* pRValsToDecRefFree;
	int maxRValsToDecRef;
	int numRValsToDecRef;
	RefDynamicArrayOfRValue** pArraysToFree;
	int maxArraysToFree;
	int numArraysToFree;
};

class CInstanceBase {
public:
	RValue*		yyvars;
	virtual ~CInstanceBase() {};
	RValue& GetYYVarRef(int index) {
		return InternalGetYYVarRef(index);
	} // end GetYYVarRef
	virtual  RValue& InternalGetYYVarRef(int index) = 0;
	RValue& GetYYVarRefL(int index) {
		return InternalGetYYVarRefL(index);
	} // end GetYYVarRef
	virtual  RValue& InternalGetYYVarRefL(int index) = 0;
};

//void GetOwnPropertyFunc(YYObjectBase * , RValue * , char * )
//void DeletePropertyFunc(YYObjectBase * , RValue * , char * , bool )
//EJSRetValBool DefineOwnPropertyFunc(YYObjectBase * , char * , RValue * , bool )
typedef void(*GetOwnPropertyFunc_t)(const YYObjectBase* obj, RValue* rv, const char* name);
typedef void(*DeletePropertyFunc_t)(const YYObjectBase* obj, RValue* rv, const char* name, bool flag);
typedef EJSRetValBool(*DefineOwnPropertyFunc_t)(const YYObjectBase* obj, const char* name, RValue* rv, bool flag);
typedef EHasInstanceRetVal(*JSHasInstanceFunc_t)(YYObjectBase* obj, RValue* rv);
typedef void(*JSConstructorFunc_t)(RValue& Result, YYObjectBase* pSelf, YYObjectBase* pOther, int argc, RValue* argv);

typedef unsigned int uint32;
typedef int int32;
typedef uint32 Hash;
typedef long long int64;
typedef const char* String;
typedef unsigned char uint8;

template<typename T>
class Element {
	T    v;
	int  k;
	Hash h;
};

template<typename Tk, typename Tv, int I = 3>
class CHashMap {
	int m_curSize;
	int m_numUsed;
	int m_curMask = I;
	int m_growThreshold;
	Element<Tv> m_elements;

	int GetIdealPosition(Hash _h) {
		return m_curMask & _h & INT_MAX;
	}
};

// I don't want to bundle the PCRE library so here we go
typedef void* pcre;
typedef void* pcre_extra;
// they're set to nullptr 99.99% of the time so who cares?

class YYObjectBase : public CInstanceBase {
public:
	// these will be set in the YYObjectBase generated by GM.
	virtual ~YYObjectBase() {};
	virtual bool Mark4GC(uint32* _pM, int _numObjects) { return false; }
	virtual bool MarkThisOnly4GC(uint32* _pM, int _numObjects) { return false; }
	virtual bool MarkOnlyChildren4GC(uint32* _pM, int _numObjects) { return false; }
	virtual void Free() {}
	virtual void ThreadFree(GCContext* _pGCContext) {}
	virtual void PreFree() {}


	YYObjectBase* m_pNextObject;
	YYObjectBase* m_pPrevObject;
	YYObjectBase* m_pPrototype;
	pcre m_pcre;
	pcre_extra m_pcreExtra;
	const char* m_class;
	GetOwnPropertyFunc_t m_getOwnProperty;
	DeletePropertyFunc_t m_deleteProperty;
	DefineOwnPropertyFunc_t m_defineOwnProperty;
	CHashMap<int, RValue*, 3>* m_yyvarsMap;
	CWeakRef** m_pWeakRefs;
	uint32 m_numWeakRefs;
	uint32 m_nvars;
	uint32 m_flags;
	uint32 m_capacity;
	uint32 m_visited;
	uint32 m_visitedGC;
	int32 m_GCgen;
	int32 m_GCcreationFrame;
	int m_slot;
	int m_kind;
	int m_rvalueInitType;
	int m_curSlot;

	void get(const std::string& name, RValue& out) {
		m_getOwnProperty(this, &out, name.c_str());
	}

	EJSRetValBool set(const std::string& name, RValue& val) {
		return m_defineOwnProperty(this, name.c_str(), &val, false);
	}
};

class RefDynamicArrayOfRValue : public YYObjectBase {
	int refcount;
	int flags;
	RValue* pArray;
	int64 owner;
	int visited;
	int length;
};

class CWeakRef : public YYObjectBase {
	YYObjectBase* m_pWeakRef;
};

typedef void(*TObjectCall)(RValue& Result, YYObjectBase* pSelf, YYObjectBase* pOther, int argc, RValue* argv);
typedef RValue&(*PFUNC_YYGMLScript_Internal)(YYObjectBase* pSelf, YYObjectBase* pOther, RValue& Result, int argc, RValue** argv);
typedef void(*PFUNC_YYGML)(YYObjectBase* pSelf, YYObjectBase* pOther);

enum eGML_TYPE : uint32 {
	eGMLT_NONE = 0,
	eGMLT_DOUBLE = 1,
	eGMLT_STRING = 2,
	eGMLT_INT32 = 4,
	eGMLT_ERROR = 0xFFFFFFFF
};

struct RToken {
	int kind;
	eGML_TYPE type;
	int ind;
	int ind2;
	RValue value;
	int itemnumb;
	RToken* items;
	int position;
};

struct YYGMLFuncs {
	char* name;
	PFUNC_YYGML func;
};

union CScriptU {
	const char* s_script;
	int s_compiledIndex;
};

class VMBuffer {
	virtual ~VMBuffer() {};
public:
	int m_size;
	int m_numLocalVarsUsed;
	int m_numArguments;
	uint8* m_pBuffer;
	void** m_pConvertedBuffer;
	int* m_pJumpBuffer;
};

class CCode {
	virtual ~CCode() {};
public:
	CCode* m_pNext;
	int i_kind;
	bool i_compiled;
	String i_str;
	RToken i_token;
	RValue i_value;
	VMBuffer* i_pVM;
	VMBuffer* i_pVMDebugInfo;
	char* i_pCode;
	char* i_pName;
	int i_CodeIndex;
	YYGMLFuncs* i_pFunc;
	bool i_watch;
	int i_offset;
	int i_locals;
	int i_args;
	int i_flags;
	YYObjectBase* i_pPrototype;
};

class CScript {
	virtual ~CScript() {};
public:
	void* s_text;
	void* s_code;
	YYGMLFuncs* s_pFunc;
	CScriptU u;
	char* s_name;
	int s_offset;
};

class CScriptRef : public YYObjectBase {
	virtual ~CScriptRef() {};
public:
	CScript* m_callScript;
	TObjectCall m_callCpp;
	PFUNC_YYGMLScript_Internal m_callYYC;
	RValue m_scope;
	RValue m_boundThis;
	YYObjectBase* m_pStatic;
	JSHasInstanceFunc_t m_hasInstance;
	JSConstructorFunc_t m_construct;
	const char* m_tag;
};

#endif /* _YOYO_H_ */
