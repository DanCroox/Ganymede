#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <format>

namespace Ganymede
{
#define GLCall(c) c;

	struct ClassID
	{
		ClassID() = default;
		ClassID(const std::type_index& typeIndex)
		{
			m_ClassID = typeIndex.hash_code();
#ifndef GM_RETAIL
			m_DebugName = typeIndex.name();
#endif // !GM_RETAIL
		}

		bool operator==(const ClassID& other) const
		{
			return m_ClassID == other.m_ClassID;
		}

		inline bool IsValid() const { return m_ClassID != -1; }

		size_t m_ClassID = static_cast<size_t>(-1);
#ifndef GM_RETAIL
	private:
		std::string m_DebugName = "None";
#endif // !GM_RETAIL
	};
}

namespace std
{
	template <>
	struct std::hash<Ganymede::ClassID>
	{
		std::size_t operator()(const Ganymede::ClassID& id) const
		{
			return std::hash<size_t>()(id.m_ClassID);
		}
	};
}

namespace Ganymede
{
#define GM_GENERATE_CLASS_ID(VarName, Class) static const ClassID VarName(std::type_index(typeid(Class)));
#define GM_GENERATE_STATIC_CLASS_ID(Class) static ClassID GetStaticClassID() { GM_GENERATE_CLASS_ID(id, Class); return id; }

	// Make sure to implement all platform interfaces in the "Platform" folder for supported platforms
#ifdef GM_PLATFORM_WINDOWS
#ifdef GM_BUILD_DLL
#define GANYMEDE_API __declspec(dllexport)
#else
#define GANYMEDE_API __declspec(dllimport)
#endif // GM_BUILD_DLL
#else
#error Ganymede only supported on Windows!
#endif // GM_PLATFORM_WINDOWS

#define BIT(x) (1 << x)

#ifndef GM_RETAIL
#define GM_CORE_ASSERTS_ENABLED
#endif // GM_RETAIL

#ifdef GM_CORE_ASSERTS_ENABLED
#define GM_CORE_ASSERT(condition, ...)													\
		{																				\
			if (!(condition))															\
			{																			\
				std::cerr << "Assertion failed: " << #condition << std::endl;			\
				std::cerr << "  file    : " << __FILE__ << std::endl;					\
				std::cerr << "  line    : " << __LINE__	<< std::endl;					\
				std::cerr << "  message : " << std::format(__VA_ARGS__) << std::endl;	\
				__debugbreak();															\
			}																			\
		}
// Assert for client applications
#define GM_ASSERT(condition, ...) GM_CORE_ASSERT(condition, __VA_ARGS__)
#else
#define GM_CORE_ASSERT(condition, message)
#define GM_ASSERT(condition, message)
#endif

// This is a tiny reflection system to obtain class inheritance hierarchy during runtime. Use when required. Not all classes need to carry this information.
#define GM_GENERATE_CLASSTYPEINFO(classname, parentClassName)													\
	inline static const ClassTypeInfoImpl<classname, parentClassName>& GetStaticClassTypeInfo()				\
	{																											\
		static const ClassTypeInfoImpl<classname, parentClassName> classTypeInfo;										\
		return classTypeInfo;																					\
	};																											\
	const ClassTypeInfo& GetClassTypeInfo() const override { return classname ##::GetStaticClassTypeInfo(); }	\


#define GM_GENERATE_BASE_CLASSTYPEINFO(classname)																\
	inline static const ClassTypeInfoImpl<classname, ClassTypeInfo>& GetStaticClassTypeInfo()				\
	{																											\
		static const ClassTypeInfoImpl<classname, ClassTypeInfo> classTypeInfo;										\
		return classTypeInfo;																					\
	};																											\
	virtual const ClassTypeInfo& GetClassTypeInfo() const = 0;													\

	struct ClassTypeInfo
	{
		ClassTypeInfo() = default;
		~ClassTypeInfo() = default;

		ClassTypeInfo(const ClassTypeInfo&) = delete;
		ClassTypeInfo& operator=(const ClassTypeInfo&) = delete;

		virtual const ClassTypeInfo* GetParentClassInfoType() const	{ return nullptr; }

		inline bool IsSubClassOf(const ClassTypeInfo& classTypeInfo) const
		{
			const ClassID otherClassID = classTypeInfo.GetClassID();
			const ClassTypeInfo* parentClassTypeInfo = GetParentClassInfoType();
			while (parentClassTypeInfo->GetClassID().IsValid())
			{
				const ClassID currentCID = parentClassTypeInfo->GetClassID();
				if (currentCID == otherClassID)
				{
					return true;
				}
				parentClassTypeInfo = parentClassTypeInfo->GetParentClassInfoType();
			}
			return false;
		}

		virtual ClassID GetClassID() const { return ClassID(); }

		static const ClassTypeInfo& GetStaticClassTypeInfo()
		{
			static ClassTypeInfo classTypeInfo;
			return classTypeInfo;
		}
	};

	template <class Clazz, class ParentClazz>
	struct ClassTypeInfoImpl : public ClassTypeInfo
	{
		const ClassTypeInfo* GetParentClassInfoType() const override
		{
			// Some minor compile time checks. Still needs to ensure classes implement the neede template correctly!
			static_assert(!std::is_same<Clazz, ParentClazz>::value && (std::is_same<ParentClazz, ClassTypeInfo>::value || std::is_base_of<ParentClazz, Clazz>::value), "Clazz must be direct subclass of ParentClazz");
			return &ParentClazz::GetStaticClassTypeInfo();
		}

		ClassID GetClassID() const override
		{
			GM_GENERATE_CLASS_ID(id, Clazz);
			return id;
		}
	};
}