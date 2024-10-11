#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <typeindex>

namespace Ganymede
{
    typedef size_t ClassID;
    static constexpr const ClassID g_InvalidClassID = -1;

#define GM_GENERATE_CLASS_ID(VarName, Class) static const ClassID id = static_cast<ClassID>(std::type_index(typeid(Class)).hash_code());
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
#define GM_CORE_ASSERT(condition, message)									\
		{																		\
			if (!(condition))													\
			{																	\
				std::cerr << "Assertion failed: " << #condition << std::endl;	\
				std::cerr << "  file    : " << __FILE__ << std::endl;			\
				std::cerr << "  line    : " << __LINE__	<< std::endl;			\
				std::cerr << "  message : " << message << std::endl;			\
				__debugbreak();													\
			}																	\
		}
#else
#define GM_CORE_ASSERT(condition, message)
#endif

// This is a tiny reflection system to obtain class inheritance hierarchy during runtime. Use when required. Not all classes need to carry this information.
#define GM_GENERATE_CLASSTYPEINFO(classname, parentClassName) inline static ClassTypeInfoImpl<classname, parentClassName> m_ClassInfoType;
    struct ClassTypeInfo
    {
        ClassTypeInfo() = default;
        ~ClassTypeInfo() = default;

        virtual const ClassTypeInfo* GetParentClassInfoType() const
        {
            return nullptr;
        }

        virtual ClassID GetClassID() const { return g_InvalidClassID; }

        static ClassTypeInfo s_ClassInfoType;
    };

    template <class Clazz, class ParentClazz>
    struct ClassTypeInfoImpl : public ClassTypeInfo
    {
        const ClassTypeInfo* GetParentClassInfoType() const override
        {
            // Some minor compile time checks. Still needs to ensure classes implement the neede template correctly!
            static_assert(!std::is_same<Clazz, ParentClazz>::value && (std::is_same<ParentClazz, ClassTypeInfo>::value || std::is_base_of<ParentClazz, Clazz>::value), "Clazz must be direct subclass of ParentClazz");
            return &ParentClazz::s_ClassInfoType;
        }

        ClassID GetClassID() const override
        {
            GM_GENERATE_CLASS_ID(id, Clazz);
            return id;
        }
    };
}