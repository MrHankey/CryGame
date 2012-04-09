////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2010.
// -------------------------------------------------------------------------
//  File name:   IFlashUI.h
//  Version:     v1.00
//  Created:     10/9/2010 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#include DEVIRTUALIZE_HEADER_FIX(IFlashUI.h)

#ifndef __IFlashUI__h__
#define __IFlashUI__h__

#include <CryExtension/ICryUnknown.h>
#include <CryExtension/CryCreateClassInstance.h>
#include <IFlashPlayer.h>
#include <IFlowSystem.h>
#include <CryName.h>
#include "functor.h"

#define IFlashUIExtensionName "FlashUI"

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// UI variant data /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef NTypelist::CConstruct<
int,
float,
EntityId,
Vec3,
string,
wstring,
bool
>::TType TUIDataTypes;

//	Default conversion uses C++ rules.
template <class From, class To>
struct SUIConversion
{
	static ILINE bool ConvertValue( const From& from, To& to )
	{
		to = (To)from;
		return true;
	}
};

//	same type conversation
#define UIDATA_NO_CONVERSION(T) \
	template <> struct SUIConversion<T,T> { \
	static ILINE bool ConvertValue( const T& from, T& to ) { to = from; return true; } \
}
UIDATA_NO_CONVERSION(int);
UIDATA_NO_CONVERSION(float);
UIDATA_NO_CONVERSION(EntityId);
UIDATA_NO_CONVERSION(Vec3);
UIDATA_NO_CONVERSION(string);
UIDATA_NO_CONVERSION(wstring);
UIDATA_NO_CONVERSION(bool);
#undef FLOWSYSTEM_NO_CONVERSION

//	Specialization for converting to bool to avoid compiler warnings.
template <class From>
struct SUIConversion<From, bool>
{
	static ILINE bool ConvertValue( const From& from, bool& to )
	{
		to = (from != 0);
		return true;
	}
};

//	Strict conversation from float to int
template <>
struct SUIConversion<float, int>
{
	static ILINE bool ConvertValue( const float& from, int& to )
	{
		int tmp = (int) from;
		if ( fabs(from - (float) tmp) < FLT_EPSILON)
		{
			to = tmp;
			return true;
		}
		return false;
	}
};

//	Vec3 conversions...
template <class To>
struct SUIConversion<Vec3, To>
{
	static ILINE bool ConvertValue( const Vec3& from, To& to )
	{
		return SUIConversion<float, To>::ConvertValue( from.x, to );
	}
};

template <class From>
struct SUIConversion<From, Vec3>
{
	static ILINE bool ConvertValue( const From& from, Vec3& to )
	{
		float temp;
		if (!SUIConversion<From, float>::ConvertValue( from, temp ))
			return false;
		to.x = to.y = to.z = temp;
		return true;
	}
};

template <>
struct SUIConversion<Vec3, bool>
{
	static ILINE bool ConvertValue( const Vec3& from, bool& to )
	{
		to = from.GetLengthSquared() > 0;
		return true;
	}
};

//	String conversions...
#define UIDATA_STRING_CONVERSION(strtype,type,fmt,fct) \
	template <> \
struct SUIConversion<type, CryStringT<strtype> > \
{ \
	static ILINE bool ConvertValue( const type& from, CryStringT<strtype>& to ) \
{ \
	to.Format( fmt, from ); \
	return true; \
} \
}; \
	template <> \
struct SUIConversion<CryStringT<strtype>, type> \
{ \
	static ILINE bool ConvertValue( const CryStringT<strtype>& from, type& to ) \
{ \
	strtype *pEnd; \
	to = fct; \
	return from.size() > 0 && *pEnd == '\0'; \
} \
};

#define SINGLE_FCT(fct) (float) fct (from.c_str(),&pEnd)
#define DOUBLE_FCT(fct) fct (from.c_str(),&pEnd,10)

UIDATA_STRING_CONVERSION(char, int, "%d", DOUBLE_FCT(strtol) );
UIDATA_STRING_CONVERSION(char, float, "%f", SINGLE_FCT(strtod) );
UIDATA_STRING_CONVERSION(char, EntityId, "%u", DOUBLE_FCT(strtoul) );

UIDATA_STRING_CONVERSION(wchar_t, int, L"%d", DOUBLE_FCT(wcstol) );
UIDATA_STRING_CONVERSION(wchar_t, float, L"%f", SINGLE_FCT(wcstod) );
UIDATA_STRING_CONVERSION(wchar_t, EntityId, L"%u", DOUBLE_FCT(wcstoul) );

#undef UIDATA_STRING_CONVERSION
#undef SINGLE_FCT
#undef DOUBLE_FCT

template <>
struct SUIConversion<bool, string>
{
	static ILINE bool ConvertValue( const bool& from, string& to )
	{
		to.Format( "%d", from );
		return true;
	}	
};

template <>
struct SUIConversion<string, bool>
{
	static ILINE bool ConvertValue( const string& from, bool& to )
	{
		int to_i;
		if ( SUIConversion<string, int>::ConvertValue(from, to_i) )
		{
			to = !!to_i;
			return true;
		}
		if (0 == stricmp (from.c_str(), "true"))
		{
			to = true;
			return true;
		}
		if (0 == stricmp (from.c_str(), "false"))
		{
			to = false;
			return true;
		}
		return false;
	}	
};

template <>
struct SUIConversion<Vec3, string>
{
	static ILINE bool ConvertValue( const Vec3& from, string& to )
	{
		to.Format( "%f,%f,%f", from.x, from.y, from.z );
		return true;
	}
};

template <>
struct SUIConversion<string, Vec3>
{
	static ILINE bool ConvertValue( const string& from, Vec3& to )
	{
		return 3 == sscanf( from.c_str(), "%f,%f,%f", &to.x, &to.y, &to.z );
	}
};

template <>
struct SUIConversion<bool, wstring>
{
	static ILINE bool ConvertValue( const bool& from, wstring& to )
	{
		to.Format( L"%d", from );
		return true;
	}	
};

template <>
struct SUIConversion<wstring, bool>
{
	static ILINE bool ConvertValue( const wstring& from, bool& to )
	{
		int to_i;
		if ( SUIConversion<wstring, int>::ConvertValue(from, to_i) )
		{
			to = !!to_i;
			return true;
		}
		if (0 == wcsicmp (from.c_str(), L"true"))
		{
			to = true;
			return true;
		}
		if (0 == wcsicmp (from.c_str(), L"false"))
		{
			to = false;
			return true;
		}
		return false;
	}	
};

template <>
struct SUIConversion<Vec3, wstring>
{
	static ILINE bool ConvertValue( const Vec3& from, wstring& to )
	{
		to.Format( L"%f,%f,%f", from.x, from.y, from.z );
		return true;
	}
};

template <>
struct SUIConversion<wstring, Vec3>
{
	static ILINE bool ConvertValue( const wstring& from, Vec3& to )
	{
		return 3 == swscanf( from.c_str(), L"%f,%f,%f", &to.x, &to.y, &to.z );
	}
};

template <>
struct SUIConversion<string, wstring>
{
	static ILINE bool ConvertValue( const string& from, wstring& to )
	{
		size_t len = from.length();
		wchar_t* buff = new wchar_t[len + 1];
		buff[len] = '\0';
		mbstowcs(buff, from.c_str(), len);
		to = buff;
		delete[] buff;
		return true;
	}
};

template <>
struct SUIConversion<wstring, string>
{
	static ILINE bool ConvertValue( const wstring& from, string& to )
	{
		size_t len = from.length();
		char* buff = new char[len + 1];
		buff[len] = '\0';
		size_t res = wcstombs(buff, from.c_str(), len);
		to = buff;
		delete[] buff;
		return res == len;
	}
};


enum EUIDataTypes
{
	eUIDT_Any = -1,
	eUIDT_Int =				NTypelist::IndexOf<int, TUIDataTypes>::value,
	eUIDT_Float =			NTypelist::IndexOf<float, TUIDataTypes>::value,
	eUIDT_EntityId =	NTypelist::IndexOf<EntityId, TUIDataTypes>::value,
	eUIDT_Vec3 =			NTypelist::IndexOf<Vec3, TUIDataTypes>::value,
	eUIDT_String =		NTypelist::IndexOf<string, TUIDataTypes>::value,
	eUIDT_WString =		NTypelist::IndexOf<wstring, TUIDataTypes>::value,
	eUIDT_Bool =			NTypelist::IndexOf<bool, TUIDataTypes>::value,
};

typedef CConfigurableVariant<TUIDataTypes, sizeof(void*), SUIConversion> TUIData;


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// UI Arguments //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#define UIARGS_DEFAULT_DELIMITER "|"

struct SUIArguments
{
	SUIArguments() : m_cDelimiter( UIARGS_DEFAULT_DELIMITER ), m_Dirty(eBDF_Delimiter) {};
	template <class T>
	SUIArguments( const T* argStringList, bool bufferStr = false ) : m_cDelimiter( UIARGS_DEFAULT_DELIMITER ) { SetArguments( argStringList, bufferStr ); }
	SUIArguments( const SFlashVarValue* vArgs, int iNumArgs ) : m_cDelimiter( UIARGS_DEFAULT_DELIMITER) { SetArguments( vArgs, iNumArgs ); }
	SUIArguments( const TUIData& data ) : m_cDelimiter( UIARGS_DEFAULT_DELIMITER ) { AddArgument( data ); }

	template <class T>
	void SetArguments( const T* argStringList, bool bufferStr = false )
	{
		Clear();
		AddArguments( argStringList, bufferStr );
	}


	template <class T>
	void AddArguments( const T* argStringList, bool bufferStr = false )
	{
		const CryStringT<T>& delimiter = GetDelimiter<T>();
		const T* delimiter_str = delimiter.c_str();
		const int delimiter_len = delimiter.length();
		const T* found = argStringList;
		while ( *found ) 
		{
			const T* next = StrStrTmpl(found, delimiter_str);
			if ( next )
			{
				((T*)next)[0] = 0;
				AddArgument( found );
				((T*)next)[0] = delimiter_str[0];
				found = next + delimiter_len;
			}
			else
			{
				AddArgument( found );
				break;
			}
		}
		if (bufferStr)
			setStringBuffer( argStringList );
	}

	void SetArguments( const SFlashVarValue* vArgs, int iNumArgs )
	{
		Clear();
		AddArguments( vArgs, iNumArgs );
	}

	void AddArguments( const SFlashVarValue* vArgs, int iNumArgs )
	{
		m_ArgList.reserve( m_ArgList.size() + iNumArgs );
		for (int i = 0; i < iNumArgs; ++i)
		{
			switch( vArgs[i].GetType() )
			{
			case SFlashVarValue::eBool:					AddArgument( vArgs[i].GetBool() );						break;
			case SFlashVarValue::eInt:					AddArgument( vArgs[i].GetInt() );							break;
			case SFlashVarValue::eUInt:					AddArgument( vArgs[i].GetUInt() );						break;
			case SFlashVarValue::eFloat:				AddArgument( vArgs[i].GetFloat() );						break;
			case SFlashVarValue::eDouble:				AddArgument( (float) vArgs[i].GetDouble() );	break;
			case SFlashVarValue::eConstStrPtr:	AddArgument( vArgs[i].GetConstStrPtr() );			break;
			case SFlashVarValue::eConstWstrPtr:	AddArgument( vArgs[i].GetConstWstrPtr() );		break;
			case SFlashVarValue::eNull:					AddArgument( "NULL" );												break;
			case SFlashVarValue::eObject:				AddArgument( "OBJECT" );											break;
			case SFlashVarValue::eUndefined:		AddArgument( "UNDEFINED" );										break;
			}
		}
	}

	template< class T >
	inline void AddArgument( const T& arg )
	{
		m_ArgList.push_back( TUIData( arg ) );
		m_Dirty = eBDF_ALL;
	}

	template< class T >
	inline void AddArgument( const T* str )
	{
		AddArgument( CryStringT<T>(str) );
	}


	inline void Clear()
	{
		m_ArgList.clear();
		m_Dirty = eBDF_ALL;
	}

	template< class T >
	static SUIArguments Create( const T& arg )
	{
		SUIArguments args;
		args.AddArgument(arg);
		return args;
	}

	template< class T >
	static SUIArguments Create( const T* str )
	{
		SUIArguments args;
		args.AddArgument(str);
		return args;
	}

	inline int GetArgCount() const { return m_ArgList.size(); }

	const char* GetAsString() const { return updateStringBuffer( m_sArgStringBuffer, eBDF_String ); }
	const wchar_t* GetAsWString() const { return updateStringBuffer( m_sArgWStringBuffer, eBDF_WString ); }
	const SFlashVarValue* GetAsList() const { return updateFlashBuffer(); }

	inline const TUIData& GetArg( int index ) const
	{
		assert( index >= 0 && index < m_ArgList.size() );
		return m_ArgList[index];
	}

	template < class T >
	inline bool GetArg( int index, T &val ) const
	{
		if ( index >= 0 && index < m_ArgList.size() )
			return m_ArgList[index].GetValueWithConversion( val );
		return false;
	}

	inline void SetDelimiter( const string& delimiter ) 
	{ 
		if ( delimiter != m_cDelimiter )
		{
			m_Dirty |= eBDF_String | eBDF_WString | eBDF_Delimiter;
		}
		m_cDelimiter = delimiter; 
	}

	template < class T >
	inline const T* StrStrTmpl( const T* str1, const T* str2 )
	{
		return strstr(str1, str2);
	}

private:
	DynArray< TUIData > m_ArgList;
	mutable string m_sArgStringBuffer;	// buffer for const char* GetAsString()
	mutable wstring m_sArgWStringBuffer;	// buffer for const wchar_t* GetAsWString()
	mutable DynArray< SFlashVarValue > m_FlashValueBuffer; // buffer for const SFlashVarValue* GetAsList()
	string m_cDelimiter;
	
	enum EBufferDirtyFlag
	{
		eBDF_None      = 0x00,
		eBDF_String    = 0x01,
		eBDF_WString   = 0x02,
		eBDF_FlashVar  = 0x04,
		eBDF_Delimiter = 0x08,
		eBDF_ALL       = 0xFF,
	};
	mutable uint m_Dirty;

	inline SFlashVarValue* updateFlashBuffer() const
	{
		if (m_Dirty & eBDF_FlashVar)
		{
			m_Dirty &= ~eBDF_FlashVar;
			m_FlashValueBuffer.clear();
			for (DynArray< TUIData >::const_iterator it = m_ArgList.begin(); it != m_ArgList.end(); ++it)
			{
				bool bConverted = false;
				switch (it->GetType())
				{
				case eUIDT_Bool:
					AddValue<bool>(*it);
					break;
				case eUIDT_Int:
					AddValue<int>(*it);
					break;
				case eUIDT_EntityId:
					AddValue<EntityId>(*it);
					break;
				case eUIDT_Float:
					{
						bool bRes = TryAddValue<int>(*it)
							|| AddValue<float>(*it);
							assert(bRes);
					}
					break;
				case eUIDT_String:
					{
						bool bRes = TryAddValue<int>(*it)
							|| TryAddValue<float>(*it)
							|| AddValue<string>(*it);
							assert(bRes);
					}
					break;
				case eUIDT_WString:
					{
						bool bRes = TryAddValue<int>(*it)
							|| TryAddValue<float>(*it)
							|| AddValue<wstring>(*it);
							assert(bRes);
					}
					break;
				case eUIDT_Any:
				default:
					assert(false);	// not supported since string->wstring or vice versa conversation 
													// would need to make a copy of data ptr and buffer somewhere. 
													// SFlashVarValue only stores char/wchar_t* ptr
					break;
				}
			}
		}
		return m_FlashValueBuffer.size() > 0 ? &m_FlashValueBuffer[0] : NULL;
	}

	template < class T >
	inline bool AddValue( const TUIData& data ) const
	{
		const T* val = data.GetPtr<T>();
		assert(val);
		m_FlashValueBuffer.push_back( SFlashVarValue(*val) );
		return true;
	}

	template < class T >
	inline bool TryAddValue( const TUIData& data ) const
	{
		T val;
		if ( data.GetValueWithConversion( val ) )
		{
			m_FlashValueBuffer.push_back( SFlashVarValue(val) );
			return true;
		}
		return false;
	}

	template <class T>
	inline const T* updateStringBuffer( CryStringT<T>& buffer, uint flag ) const
	{
		if (m_Dirty & flag)
		{
			m_Dirty &= ~flag;
			CryStringT<T> delimiter_str = GetDelimiter<T>();
			buffer.clear();
			for ( DynArray< TUIData >::const_iterator it = m_ArgList.begin(); it != m_ArgList.end(); ++it )
			{
				if (buffer.size() > 0) buffer += delimiter_str;
				CryStringT<T> val;
				bool bRes = it->GetValueWithConversion(val);
				assert(bRes && "try to convert to char* string list but some of the values are unsupported wchar_t*");
				buffer += val;
			}
		}
		return buffer.c_str();
	}

	template <class T>
	inline const CryStringT<T>& GetDelimiter() const
	{
		static CryStringT<T> delimiter_str;
		if (m_Dirty & eBDF_Delimiter)
		{
			m_Dirty &= ~eBDF_Delimiter;
			TUIData delimiter(m_cDelimiter);
			delimiter.GetValueWithConversion(delimiter_str);
		}
		return delimiter_str;
	}

	template <class T>
	inline void setStringBuffer( const T* str ) { assert(false); }
};

// Specialize in global scope
template <>
inline const CryStringT<char>& SUIArguments::GetDelimiter() const
{
	return m_cDelimiter;
}

template <>
inline void SUIArguments::setStringBuffer( const char* str )
{
	m_sArgStringBuffer = str;
	m_Dirty &= ~eBDF_String;
}

template <>
inline void SUIArguments::setStringBuffer( const wchar_t* str )
{
	m_sArgWStringBuffer = str;
	m_Dirty &= ~eBDF_WString;
}

template <>
inline const wchar_t* SUIArguments::StrStrTmpl( const wchar_t* str1, const wchar_t* str2 )
{
	return wcsstr(str1, str2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// UI Descriptions /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SUIParameterDesc
{
	enum EUIParameterType
	{
		eUIPT_Any,
		eUIPT_Bool,
		eUIPT_Int,
		eUIPT_Float,
		eUIPT_String,
	};

	SUIParameterDesc() : sName("undefined"), sDisplayName("undefined"), sDesc("undefined"), eType(eUIPT_Any) {} 
	SUIParameterDesc( const char*  name, const char*  displ, const char*  desc, EUIParameterType type = eUIPT_Any) : sName(name), sDisplayName(displ), sDesc(desc), eType(type) {}
	const char* sName;
	const char* sDisplayName;
	const char* sDesc;
	EUIParameterType eType;

	inline bool operator==( const SUIParameterDesc& other ) const
	{
		return strcmp(sName, other.sName) == 0;
	}
};
typedef DynArray< SUIParameterDesc > TUIParams;

struct SUIEventDesc : public SUIParameterDesc
{
	SUIEventDesc() : IsDynamic(false), sDynamicName("Array"), sDynamicDesc("") {}
	SUIEventDesc( const char*  name, const char*  displ, const char*  desc, bool isDyn = false, const char* dynName="Array", const char* dynDesc="" ) : SUIParameterDesc( name, displ, desc ), IsDynamic(isDyn), sDynamicName(dynName), sDynamicDesc(dynDesc) {}
	TUIParams Params;
	bool IsDynamic;
	const char* sDynamicName;
	const char* sDynamicDesc;

	inline bool operator==( const SUIEventDesc& other ) const
	{		
		bool res = strcmp(sName, other.sName) == 0 && IsDynamic == other.IsDynamic && Params.size() == other.Params.size();
		for ( int i = 0; i < Params.size() && res; ++i)
		{
			res &= Params[i] == other.Params[i];
		}
		return res;
	}

	inline void SetDynamic( const char* name, const char* desc )
	{
		IsDynamic = true;
		sDynamicName = name;
		sDynamicDesc = desc;
	}
};
typedef DynArray< SUIEventDesc > TUIEvents;

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// UI Element ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
struct IUIElement;

struct IUIElementEventListener
{
	virtual void OnUIEvent( IUIElement* pSender, const SUIEventDesc& event, const SUIArguments& args ) {}
	virtual void OnUIEventEx( IUIElement* pSender, const char* fscommand, const SUIArguments& args, void* pUserData ) {}

	virtual void OnInit( IUIElement* pSender, IFlashPlayer* pFlashPlayer ) {}
	virtual void OnUnload( IUIElement* pSender) {}
	virtual void OnSetVisible(  IUIElement* pSender, bool bVisible ) {}

	virtual void OnInstanceCreated( IUIElement* pSender, IUIElement* pNewInstance ) {}
	virtual void OnInstanceDestroyed( IUIElement* pSender, IUIElement* pDeletedInstance ) {}
protected:
  virtual ~IUIElementEventListener() {}; 
};

UNIQUE_IFACE struct IUIElementIterator
{
	virtual ~IUIElementIterator() {}

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual IUIElement* Next() = 0;
	virtual int GetCount() const = 0;
};
TYPEDEF_AUTOPTR(IUIElementIterator);
typedef IUIElementIterator_AutoPtr IUIElementIteratorPtr;

UNIQUE_IFACE struct IUIElement
{
	struct SUIConstraints
	{
		enum EPositionType
		{
			ePT_Fixed,
			ePT_Fullscreen,
			ePT_Dynamic
		};

		enum EPositionAlign
		{
			ePA_Lower,
			ePA_Mid,
			ePA_Upper,
		};

		SUIConstraints() 
			: eType(ePT_Dynamic)
			, iLeft(0)
			, iTop(0)
			, iWidth(1024)
			, iHeight(768)
			, eHAlign(ePA_Mid)
			, eVAlign(ePA_Mid)
			, bScale(true)
		{
		}

		SUIConstraints( EPositionType type, int left, int top, int width, int height, EPositionAlign halign, EPositionAlign valign, bool scale, bool ismax )
			: eType(type)
			, iLeft(left)
			, iTop(top)
			, iWidth(width)
			, iHeight(height)
			, eHAlign(halign)
			, eVAlign(valign)
			, bScale(scale)
			, bMax(ismax)
		{
		}

		EPositionType eType;
		int iLeft;
		int iTop;
		int iWidth;
		int iHeight;
		EPositionAlign eHAlign;
		EPositionAlign eVAlign;
		bool bScale;
		bool bMax;
	};

	enum EFlashUIFlags
	{
		// flags per instance
		eFUI_HARDWARECURSOR    = 0x0001,
		eFUI_MOUSEEVENTS       = 0x0002,
		eFUI_KEYEVENTS         = 0x0004,
		eFUI_CONSOLE_MOUSE     = 0x0008,
		eFUI_CONSOLE_CURSOR    = 0x0010,
		eFUI_CONTROLLER_INPUT  = 0x0020,
		eFUI_EVENTS_EXCLUSIVE  = 0x0040,
		eFUI_RENDER_LOCKLESS   = 0x0080,
		eFUI_FIXED_PROJ_DEPTH  = 0x0100,
		eFUI_MASK_PER_INSTANCE = 0x0FFF,

		// flags per UIElement
		eFUI_FORCE_NO_UNLOAD   = 0x1000,
		eFUI_MASK_PER_ELEMENT  = 0xF000,
	};

	enum EControllerInputEvent
	{
		eCIE_Up = 0,
		eCIE_Down,
		eCIE_Left,
		eCIE_Right,
		eCIE_Action,
		eCIE_Back,
		eCIE_Start,
		eCIE_Select,
		eCIE_ShoulderL1,
		eCIE_ShoulderL2,
		eCIE_ShoulderR1,
		eCIE_ShoulderR2,
		eCIE_Button3,
		eCIE_Button4,
	};

	enum EControllerInputState
	{
		eCIS_OnPress = 0,
		eCIS_OnRelease,
	};

	virtual ~IUIElement() {}

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	// instances
	virtual uint GetInstanceID() const = 0;
	virtual IUIElement* GetInstance( uint instanceID ) = 0;
	virtual IUIElementIteratorPtr GetInstances() const = 0;
	virtual bool DestroyInstance( uint instanceID ) = 0;
	virtual bool DestroyThis() = 0;

	// common
	virtual void SetName( const char* name ) = 0;
	virtual const char* GetName() const = 0;

	virtual void SetGroupName( const char* groupName ) = 0;
	virtual const char* GetGroupName() const = 0;

	virtual void SetFlashFile( const char* flashFile ) = 0;
	virtual const char* GetFlashFile() const = 0;

	virtual bool Init( bool bLoadAsset = true ) = 0;
	virtual void Unload( bool bAllInstances = false ) = 0; 
	virtual void Reload( bool bAllInstances = false ) = 0;
	virtual bool IsInit() const = 0;

	virtual bool IsDeleted() const = 0;

	virtual void UnloadBootStrapper() = 0;
	virtual void ReloadBootStrapper() = 0;

	virtual void Update( float fDeltaTime ) = 0;
	virtual void Render() = 0;
	virtual void RenderLockless() = 0;

	// visibility
	virtual void RequestHide() = 0;

	virtual void SetVisible( bool bVisible ) = 0;
	virtual bool IsVisible() const = 0;

	virtual void SetFlag( EFlashUIFlags flag, bool bSet ) = 0;
	virtual bool HasFlag( EFlashUIFlags flag ) const = 0;

	virtual float GetAlpha() const = 0;
	virtual void SetAlpha( float fAlpha ) = 0;

	virtual int GetLayer() const = 0;
	virtual void SetLayer( int iLayer ) = 0;

	virtual void SetConstraints( const SUIConstraints& newConstraints ) = 0;
	virtual const IUIElement::SUIConstraints& GetConstraints() const = 0;

	// raw IFlashPlayer
	virtual IFlashPlayer* GetFlashPlayer() = 0;

	// will call AddRef to the IFlashPlayer! make sure to release it once it is not needed anymore!
	virtual IFlashPlayer* GetFlashPlayerThreadSafe() = 0;

	// definitions
	virtual const SUIParameterDesc* GetVariableDesc( int index ) const = 0;
	virtual const SUIParameterDesc* GetVariableDesc( const char* varName ) const = 0;
	virtual int GetVariableCount() const = 0;

	virtual const SUIParameterDesc* GetArrayDesc( int index ) const = 0;
	virtual const SUIParameterDesc* GetArrayDesc( const char* arrayName ) const = 0;
	virtual int GetArrayCount() const = 0;

	virtual const SUIParameterDesc* GetMovieClipDesc( int index ) const = 0;
	virtual const SUIParameterDesc* GetMovieClipDesc( const char* movieClipName ) const = 0;
	virtual int GetMovieClipCount() const = 0;

	virtual const SUIParameterDesc* GetMovieClipTmplDesc( int index ) const = 0;
	virtual const SUIParameterDesc* GetMovieClipTmplDesc( const char* movieClipTmplName ) const = 0;
	virtual int GetMovieClipTmplCount() const = 0;

	virtual const SUIEventDesc* GetEventDesc( int index ) const = 0;
	virtual const SUIEventDesc* GetEventDesc( const char* eventName ) const = 0;
	virtual int GetEventCount() const = 0;

	virtual const SUIEventDesc* GetFunctionDesc( int index ) const = 0;
	virtual const SUIEventDesc* GetFunctionDesc( const char* functionName ) const = 0;
	virtual int GetFunctionCount() const = 0;

	virtual void UpdateViewPort() = 0;
	virtual void GetViewPort( int &x, int &y, int &width, int &height, float& aspectRatio ) = 0;

	virtual bool Serialize( XmlNodeRef& xmlNode, bool bIsLoading ) = 0;

	// event listener
	virtual void AddEventListener( IUIElementEventListener* pListener, const char* name ) = 0;
	virtual void RemoveEventListener( IUIElementEventListener* pListener ) = 0;

	// functions and objects
	virtual bool CallFunction( const char* fctName, const SUIArguments& args = SUIArguments(), TUIData* pDataRes = NULL ) = 0;
	virtual bool CallFunction( const SUIEventDesc* pFctDesc, const SUIArguments& args = SUIArguments(), TUIData* pDataRes = NULL ) = 0;

	virtual IFlashVariableObject* GetMovieClip( const char* movieClipName ) = 0;
	virtual IFlashVariableObject* GetMovieClip( const SUIParameterDesc* pMovieClipDesc ) = 0;

	virtual IFlashVariableObject* CreateMovieClip( SUIParameterDesc& newInstanceDesc, const char* movieClipTemplate, const SUIArguments& args = SUIArguments(), const char* mcInstanceName = NULL ) = 0;
	virtual IFlashVariableObject* CreateMovieClip( SUIParameterDesc& newInstanceDesc, const SUIParameterDesc* pMovieClipTemplateDesc, const SUIArguments& args = SUIArguments(), const char* mcInstanceName = NULL ) = 0;

	virtual bool SetVariable( const char* varName, const TUIData& value ) = 0;
	virtual bool SetVariable( const SUIParameterDesc* pVarDesc, const TUIData& value ) = 0;

	virtual bool GetVariable( const char* varName, TUIData& valueOut ) = 0;
	virtual bool GetVariable( const SUIParameterDesc* pVarDesc, TUIData& valueOut ) = 0;

	virtual bool CreateVariable( const char* varName, const TUIData& value ) = 0;

	virtual bool SetArray( const char* arrayName, const SUIArguments& values ) = 0;
	virtual bool SetArray( const SUIParameterDesc* pArrayDesc, const SUIArguments& values ) = 0;

	virtual bool GetArray( const char* arrayName, SUIArguments& valuesOut ) = 0;
	virtual bool GetArray( const SUIParameterDesc* pArrayDesc, SUIArguments& valuesOut ) = 0;

	virtual bool CreateArray( const char* arrayName, const SUIArguments& values ) = 0;

	template <class T>
	inline bool SetVar( const char* varName, const T& value)
	{
		return SetVariable( varName, TUIData(value) );
	}

	template <class T>
	inline T GetVar( const char* varName )
	{
		TUIData out;
		if ( GetVariable( varName, out ) )
		{
			T res;
			if ( out.GetValueWithConversion( res ) )
				return res;
		}
		assert(false);
		return T();
	}

	// ITexture 
	virtual void LoadTexIntoMc( const char* movieClip, ITexture* pTexture ) = 0;
	virtual void LoadTexIntoMc( const SUIParameterDesc* pMovieClipDesc, ITexture* pTexture ) = 0;

	// dynamic textures
	virtual void AddTexture( IDynTextureSource* pDynTexture ) = 0;
	virtual void RemoveTexture( IDynTextureSource* pDynTexture ) = 0;
	virtual int GetNumExtTextures() const = 0;

	// input events
	virtual void SendCursorEvent( SFlashCursorEvent::ECursorState evt, int iX, int iY, int iButton = 0, float fWheel = 0.f ) = 0;
	virtual void SendKeyEvent( const SFlashKeyEvent& evt ) = 0;
	virtual void SendCharEvent( const SFlashCharEvent& charEvent ) = 0;
	virtual void SendControllerEvent( EControllerInputEvent event, EControllerInputState state ) = 0;

	virtual void GetMemoryUsage(ICrySizer * s) const = 0;
};
typedef std::vector< IUIElement* > TUIElements;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// UI Action ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
UNIQUE_IFACE struct IUIAction
{
	virtual ~IUIAction() {}

	virtual const char* GetName() const = 0;
	virtual void SetName( const char* name ) = 0;

	virtual bool Init() = 0;
	virtual bool IsDeleted() const = 0;

	virtual IFlowGraphPtr GetFlowGraph() const = 0;

	virtual bool Serialize( XmlNodeRef& xmlNode, bool bIsLoading ) = 0;

	virtual void GetMemoryUsage(ICrySizer * s) const = 0;
};
typedef std::vector< IUIAction* > TUIActions;

struct IUIActionListener
{
	virtual void OnStart( IUIAction* pAction, const SUIArguments& args ) = 0;
	virtual void OnEnd( IUIAction* pAction, const SUIArguments& args ) = 0;
protected:
  virtual ~IUIActionListener() {}
};

UNIQUE_IFACE struct IUIActionManager
{
	virtual ~IUIActionManager() {}

	virtual void StartAction( IUIAction* pAction, const SUIArguments& args ) = 0;
	virtual void EndAction( IUIAction* pAction, const SUIArguments& args ) = 0;
	virtual void EnableAction( IUIAction* pAction, bool bEnable ) = 0;

	virtual void AddListener( IUIActionListener* pListener, const char* name ) = 0;
	virtual void RemoveListener( IUIActionListener* pListener ) = 0;

	virtual void GetMemoryUsage(ICrySizer * s) const = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// UI Events ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SUIEvent
{
	SUIEvent( uint evt, SUIArguments agruments ) : event(evt), args(agruments) {}
	uint event;
	SUIArguments args;
};

struct IUIEventListener
{
	virtual void OnEvent( const SUIEvent& event ) = 0;
protected:
  virtual ~IUIEventListener() {}
};

UNIQUE_IFACE struct IUIEventSystem
{
	enum EEventSystemType
	{
		eEST_UI_TO_SYSTEM,
		eEST_SYSTEM_TO_UI,
	};

	virtual ~IUIEventSystem() {}

	virtual const char* GetName() const = 0;
	virtual IUIEventSystem::EEventSystemType GetType() const = 0;

	virtual uint RegisterEvent( const SUIEventDesc& eventDesc ) = 0;

	virtual void RegisterListener( IUIEventListener* pListener, const char* name ) = 0;
	virtual void UnregisterListener( IUIEventListener* pListener ) = 0;

	virtual void SendEvent( const SUIEvent& event ) = 0;

	virtual const SUIEventDesc* GetEventDesc( int index ) const = 0;
	virtual const SUIEventDesc* GetEventDesc( const char* eventName ) const = 0;
	virtual int GetEventCount() const = 0;
	
	virtual uint GetEventId( const char* sEventName ) = 0;

	virtual void GetMemoryUsage(ICrySizer * s) const = 0;
};

UNIQUE_IFACE struct IUIEventSystemIterator
{
	virtual ~IUIEventSystemIterator() {}

	virtual void AddRef() = 0;
	virtual void Release() = 0;
	virtual IUIEventSystem* Next( string &name ) = 0;
};

TYPEDEF_AUTOPTR(IUIEventSystemIterator);
typedef IUIEventSystemIterator_AutoPtr IUIEventSystemIteratorPtr;

template<class T>
struct SUIEventHelper
{
	typedef void (T::*TEventFct) ( const SUIEvent& event );
	void RegisterEvent( IUIEventSystem* pEventSystem, const SUIEventDesc &event, TEventFct fct )
	{
		mFunctionMap[pEventSystem->RegisterEvent(event)] = fct;
	}
	void Dispatch( T* pThis, const SUIEvent& event )
	{
		TFunctionMapIter it = mFunctionMap.find( event.event );
		if (it != mFunctionMap.end())
			(pThis->*it->second)( event );
	}

private:
	typedef std::map<uint, TEventFct> TFunctionMap;
	typedef typename TFunctionMap::iterator TFunctionMapIter;
	TFunctionMap mFunctionMap;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// UI Interface ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
struct IVirtualKeyboardEvents;

struct IUIModule
{
	virtual ~IUIModule() {};

	// called once on initialization of the UISystem
	virtual void Init() {};

	// called once on shutdown of the UISystem
	virtual void Shutdown() {};

	// called if gfx_reload_all command was issued
	virtual void Reload() {};

	// called on FlashUI reset (unload level etc.)
	virtual void Reset() {};

	virtual void Update(float fDelta) {};
};

UNIQUE_IFACE struct IFlashUI : public ICryUnknown
{
	CRYINTERFACE_DECLARE( IFlashUI, 0xE1161004DA5B4F04, 0x9DFF8FC0EACE3BD4 )

public:
	DEVIRTUALIZATION_VTABLE_FIX

	// init the Flash UI system
	virtual void Init() = 0;
	virtual bool PostInit() = 0;

	// update the ui system
	virtual void Update(float fDeltatime) = 0;

	// reload UI xml files
	virtual void Reload() = 0;

	// shut down
	virtual void Shutdown() = 0;

	virtual bool LoadElementsFromFile( const char* fileName ) = 0;
	virtual bool LoadActionFromFile( const char* fileName ) = 0;

	// access for IUIElements
	virtual IUIElement* GetUIElement( const char* name ) const = 0;
	virtual IUIElement* GetUIElement( int index ) const = 0;
	virtual int GetUIElementCount() const = 0;

	virtual IUIElement* GetUIElementByInstanceStr( const char* UIInstanceStr) const = 0;

	// access for IUIActions
	virtual IUIAction* GetUIAction( const char* name ) const = 0;
	virtual IUIAction* GetUIAction( int index ) const = 0;
	virtual int GetUIActionCount() const = 0;

	virtual IUIActionManager* GetUIActionManager() const = 0;

	// updates all UIAction flowgraphs
	// will update all FGs in a loop until all event stacks are flushed
	virtual void UpdateFG() = 0;

	virtual void RegisterModule( IUIModule* pModule, const char* name ) = 0;
	virtual void UnregisterModule( IUIModule* pModule ) = 0;


	// only valid in editor, also only used by UI Editor to prevent stack overflow while FG is not updated.
	virtual void EnableEventStack( bool bEnable ) = 0;

	// event system to auto create flownodes for communication between flash and c++
	virtual IUIEventSystem* CreateEventSystem( const char* name, IUIEventSystem::EEventSystemType eType ) = 0;
	virtual IUIEventSystem* GetEventSystem( const char* name, IUIEventSystem::EEventSystemType eType ) = 0;
	virtual IUIEventSystemIteratorPtr CreateEventSystemIterator( IUIEventSystem::EEventSystemType eType ) = 0; 

	// input events
	virtual void DispatchControllerEvent( IUIElement::EControllerInputEvent event, IUIElement::EControllerInputState state ) = 0;
	virtual void SendFlashMouseEvent( SFlashCursorEvent::ECursorState evt, int iX, int iY, int iButton = 0, float fWheel = 0.f, bool bFromController = false ) = 0;
	virtual bool DisplayVirtualKeyboard( unsigned int flags, const wchar_t* title, const wchar_t* initialInput, int maxInputLength, IVirtualKeyboardEvents *pInCallback ) = 0;
	virtual bool IsVirtualKeyboardRunning() = 0;
	virtual bool CancelVirtualKeyboard() = 0;

	// used by scaleform to validate if texture is preloaded (only debug/profile)
	virtual void CheckPreloadedTexture(ITexture* pTexture) const = 0;

	// memory statistics
	virtual void GetMemoryStatistics(ICrySizer * s) const = 0;

	// screen size stuff for UI Emulator (Sandbox) only!
	virtual void GetScreenSize(int &width, int &height) = 0;
	typedef Functor2< int&, int& > TEditorScreenSizeCallback;
	virtual void SetEditorScreenSizeCallback(TEditorScreenSizeCallback& cb) = 0;
	virtual void RemoveEditorScreenSizeCallback() = 0;

	// plattform stuff for UI Emulator (Sandbox) only!
	enum EPlatformUI
	{
		ePUI_PC = 0,
		ePUI_X360,
		ePUI_PS3,
	};
	typedef Functor0wRet< EPlatformUI > TEditorPlatformCallback;
	virtual void SetEditorPlatformCallback(TEditorPlatformCallback& cb) = 0;
	virtual void RemoveEditorPlatformCallback() = 0;

	// logging listener for UI Emulator (Sandbox) only!
	enum ELogEventLevel
	{
		eLEL_Log = 0,
		eLEL_Warning,
		eLEL_Error,
	};

	struct SUILogEvent
	{
		string Message;
		ELogEventLevel Level;
	};

	typedef Functor1< const SUILogEvent& > TEditorUILogEventCallback;
	virtual void SetEditorUILogEventCallback(TEditorUILogEventCallback& cb) = 0;
	virtual void RemoveEditorUILogEventCallback() = 0;
};

DECLARE_BOOST_POINTERS( IFlashUI );

static IFlashUIPtr GetIFlashUIPtr()
{
	IFlashUIPtr pFlashUI;
	if (gEnv && gEnv->pSystem)
		CryCreateClassInstance(IFlashUIExtensionName, pFlashUI);
	return pFlashUI;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Lookup Table ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class Base, class IDD>
struct SUIItemLookupSet
{
	typedef std::vector< Base > TITems;
	typedef std::map< CCryName, int > TLookup;
	typedef typename TITems::iterator iterator;
	typedef typename TITems::const_iterator const_iterator;
	typedef typename TITems::reverse_iterator reverse_iterator;
	typedef typename TITems::const_reverse_iterator const_reverse_iterator;
	typedef typename TITems::size_type size_type;
	typedef typename TITems::value_type value_type;

	inline Base& operator[](int __n)
	{
		return m_Items[__n];
	}

	inline Base* operator()(CCryName __name)
	{
		typename TLookup::iterator it = m_Lookup.find(__name);
		return it != m_Lookup.end() ? &m_Items[it->second] : NULL;
	}

	inline const Base& operator[](int __n) const
	{
		return m_Items[__n];
	}

	inline const Base* operator()(CCryName __name) const
	{
		typename TLookup::const_iterator it = m_Lookup.find(__name);
		return it != m_Lookup.end() ? &m_Items[it->second] : NULL;
	}

	void push_back(const Base& item)
	{
		IDD id(item);
		assert(m_Lookup[id.Id()] == NULL);
		m_Lookup[id.Id()] = m_Items.size();
		m_Items.push_back(item);
	}

	void remove(const Base& item)
	{
		IDD id(item);
		typename TLookup::iterator it =  m_Lookup.find(id.Id());
		if (it != m_Lookup.end())
		{
			for (typename TLookup::iterator nextIt = it; nextIt != m_Lookup.end(); ++nextIt)
				nextIt->second--;
			m_Lookup.erase(it);
			for (typename TITems::iterator it = m_Items.begin(); it != m_Items.end(); ++it)
			{
				if (*it == item)
				{
					m_Items.erase(it);
					return;
				}
			}
		}
		assert(false);
	}

	void clear()
	{
		m_Items.clear();
		m_Lookup.clear();
	}

	inline size_type size() const { return m_Items.size(); }

	inline iterator begin() { return m_Items.begin(); }
	inline iterator end() { return m_Items.end(); }
	inline const_iterator begin() const { return m_Items.begin(); }
	inline const_iterator end() const { return m_Items.end(); }
	inline reverse_iterator rbegin() { return m_Items.rbegin(); }
	inline reverse_iterator rend() { return m_Items.rend(); }
	inline const_reverse_iterator rbegin() const { return m_Items.rbegin(); }
	inline const_reverse_iterator rend() const { return m_Items.rend(); }
	inline size_type capacity() const { return m_Items.capacity(); }

private:
	TITems m_Items;
	TLookup m_Lookup;
};

#define LOOKUP_IDD_PTR(type, fct) struct IDD##type { \
	IDD##type(type* p) : m_p(p) {} \
	inline const char* Id() const{ return m_p->fct; } \
	type* m_p; \
};

#define LOOKUP_IDD(type, fct) struct IDD##type { \
	IDD##type(const type& p) : m_o(p) {} \
	inline const char* Id() const{ return m_o.fct; } \
	type m_o; \
};

LOOKUP_IDD_PTR(IUIElement, GetName());
LOOKUP_IDD_PTR(IUIAction, GetName());
LOOKUP_IDD(SUIParameterDesc, sDisplayName);
LOOKUP_IDD(SUIEventDesc, sDisplayName);

typedef SUIItemLookupSet<IUIElement*, IDDIUIElement> TUIElementsLookup;
typedef SUIItemLookupSet<IUIAction*, IDDIUIAction> TUIActionsLookup;
typedef SUIItemLookupSet<SUIParameterDesc, IDDSUIParameterDesc> TUIParamsLookup;
typedef SUIItemLookupSet<SUIEventDesc, IDDSUIEventDesc> TUIEventsLookup;

// ---------------------------------------------------------------
// -------------------- per instance call ------------------------
// ---------------------------------------------------------------
template <class T>
struct SPerInstanceCallBase
{
	typedef Functor2< IUIElement*, T > TCallback;

	bool Execute(IUIElement* pBaseElement, int instanceId, const TCallback& cb, T data, bool bAllowMultiInstances = true)
	{
		CRY_ASSERT_MESSAGE( pBaseElement, "NULL pointer passed!" );
		if ( pBaseElement )
		{
			if ( instanceId < 0 )
			{
				CRY_ASSERT_MESSAGE( bAllowMultiInstances, "SPerInstanceCallBase is used on multiple instances, but bAllowMultiInstances is set to false!" );
				if (!bAllowMultiInstances)
					return false;

				IUIElementIteratorPtr instances = pBaseElement->GetInstances();
				while ( IUIElement* pInstance = instances->Next() )
				{
					// instanceId < -1 == only call instances that are initialized
					if (instanceId < -1 && !pInstance->IsInit())
						continue;

					cb(pInstance, data);
				}
			}
			else
			{
				IUIElement* pInstance = pBaseElement->GetInstance( (uint) instanceId );
				if ( pInstance )
					cb(pInstance, data);
			}
			return true;
		}
		return false;
	}
};

// ---------------------------------------------------------------
struct SPerInstanceCall0
{
	typedef Functor1< IUIElement* > TCallback;

	bool Execute(IUIElement* pBaseElement, int instanceId, const TCallback& cb, bool bAllowMultiInstances = true )
	{
		SPerInstanceCallBase< const TCallback& > base;
		return base.Execute(pBaseElement, instanceId, functor(*this, &SPerInstanceCall0::_cb), cb, bAllowMultiInstances);
	}

private:
	void _cb(IUIElement* pInstance, const TCallback& cb) { cb(pInstance); }
};

// ---------------------------------------------------------------
template <class T>
struct SPerInstanceCall1 : public SPerInstanceCallBase< T >
{
};

// ---------------------------------------------------------------
template <class T1, class T2>
struct SPerInstanceCall2
{
	typedef Functor3< IUIElement*, T1, T2 > TCallback;

	struct SCallData
	{
		SCallData(const TCallback& _cb, T1 _arg1, T2 _arg2) : cb(_cb), arg1(_arg1), arg2(_arg2) {}
		const TCallback& cb; T1 arg1; T2 arg2;
	};

	bool Execute(IUIElement* pBaseElement, int instanceId, const TCallback& cb, T1 arg1, T2 arg2, bool bAllowMultiInstances = true)
	{
		SPerInstanceCallBase< const SCallData& > base;
		return base.Execute(pBaseElement, instanceId, functor(*this, &SPerInstanceCall2::_cb), SCallData(cb, arg1, arg2), bAllowMultiInstances);
	}

private:
	void _cb(IUIElement* pInstance, const SCallData& data) { data.cb(pInstance, data.arg1, data.arg2); }
};

// ---------------------------------------------------------------
template <class T1, class T2, class T3>
struct SPerInstanceCall3
{
	typedef Functor4< IUIElement*, T1, T2, T3 > TCallback;

	struct SCallData
	{
		SCallData(const TCallback& _cb, T1 _arg1, T2 _arg2, T3 _arg3) : cb(_cb), arg1(_arg1), arg2(_arg2), arg3(_arg3) {}
		const TCallback& cb; T1 arg1; T2 arg2; T3 arg3;
	};

	bool Execute(IUIElement* pBaseElement, int instanceId, const TCallback& cb, T1 arg1, T2 arg2, T3 arg3, bool bAllowMultiInstances = true)
	{
		SPerInstanceCallBase< const SCallData& > base;
		return base.Execute(pBaseElement, instanceId, functor(*this, &SPerInstanceCall3::_cb), SCallData(cb, arg1, arg2, arg3), bAllowMultiInstances);
	}

private:
	void _cb(IUIElement* pInstance, const SCallData& data) { data.cb(pInstance, data.arg1, data.arg2, data.arg3); }
};

// ---------------------------------------------------------------
template <class T1, class T2, class T3, class T4>
struct SPerInstanceCall4
{
	typedef Functor5< IUIElement*, T1, T2, T3, T4 > TCallback;

	struct SCallData
	{
		SCallData(const TCallback& _cb, T1 _arg1, T2 _arg2, T3 _arg3, T4 _arg4) : cb(_cb), arg1(_arg1), arg2(_arg2), arg3(_arg3), arg4(_arg4) {}
		const TCallback& cb; T1 arg1; T2 arg2; T3 arg3; T4 arg4;
	};

	bool Execute(IUIElement* pBaseElement, int instanceId, const TCallback& cb, T1 arg1, T2 arg2, T3 arg3, T4 arg4, bool bAllowMultiInstances = true)
	{
		SPerInstanceCallBase< const SCallData& > base;
		return base.Execute(pBaseElement, instanceId, functor(*this, &SPerInstanceCall4::_cb), SCallData(cb, arg1, arg2, arg3, arg4), bAllowMultiInstances);
	}

private:
	void _cb(IUIElement* pInstance, const SCallData& data) { data.cb(pInstance, data.arg1, data.arg2, data.arg3, data.arg4); }
};

#endif