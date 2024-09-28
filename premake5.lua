workspace "Ganymede"
	architecture "x64"
	configurations
	{
		"Debug",
		"Release",
		"Retail"
	}
	startproject "GanymedeApp"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
bindir = "_bin/"
intermediatedir = "_intermediate/"


project "Ganymede"
	location "Ganymede"
	kind "SharedLib"
	language "C++"

	targetdir (bindir .. outputdir .. "/%{prj.name}")
	objdir (intermediatedir .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Ganymede/vendor/spdlog/include",
		"Ganymede/vendor/assimp/include",
		"Ganymede/vendor/assimp/build/include"
	}

	links
	{
		"assimp"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"GM_PLATFORM_WINDOWS",
			"GM_BUILD_DLL"
		}

		postbuildcommands
		{
 			("{MKDIR} ../" .. bindir .. outputdir .. "/GanymedeApp"),
			("{COPYFILE} %{cfg.buildtarget.relpath} ../" .. bindir .. outputdir .. "/GanymedeApp")
		}

	filter "configurations:Debug"
		defines "GM_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "GM_RELEASE"
		optimize "On"

	filter "configurations:Retail"
		defines "GM_RETAIL"
		optimize "On"


project "GanymedeApp"
	location "GanymedeApp"
	kind "ConsoleApp"
	language "C++"

	targetdir (bindir .. outputdir .. "/%{prj.name}")
	objdir (intermediatedir .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Ganymede/src",
		"Ganymede/vendor/spdlog/include"
	}

	links
	{
		"Ganymede",
		"assimp"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"GM_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "GM_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "GM_RELEASE"
		optimize "On"

	filter "configurations:Retail"
		defines "GM_RETAIL"
		optimize "On"


project "assimp"
	location "Ganymede/vendor/assimp"
	kind "StaticLib"
	language "C++"

	targetdir (bindir .. outputdir .. "/%{prj.name}")
	objdir (intermediatedir .. outputdir .. "/%{prj.name}")

	files
	{
		"Ganymede/vendor/assimp/code/Common/Assimp.cpp",
		"Ganymede/vendor/assimp/code/res/assimp.rc",
		"Ganymede/vendor/assimp/code/Common/DefaultLogger.cpp",
		"Ganymede/vendor/assimp/code/Common/FileLogStream.h",
		"Ganymede/vendor/assimp/code/Common/StdOStreamLogStream.h",
		"Ganymede/vendor/assimp/code/Common/Win32DebugLogStream.h",
		"Ganymede/vendor/assimp/code/CApi/AssimpCExport.cpp",
		"Ganymede/vendor/assimp/code/Common/Exporter.cpp",
		"Ganymede/vendor/assimp/contrib/openddlparser/code/DDLNode.cpp",
		"Ganymede/vendor/assimp/contrib/openddlparser/include/openddlparser/DDLNode.h",
		"Ganymede/vendor/assimp/contrib/openddlparser/code/OpenDDLCommon.cpp",
		"Ganymede/vendor/assimp/contrib/openddlparser/include/openddlparser/OpenDDLCommon.h",
		"Ganymede/vendor/assimp/contrib/openddlparser/code/OpenDDLExport.cpp",
		"Ganymede/vendor/assimp/contrib/openddlparser/include/openddlparser/OpenDDLExport.h",
		"Ganymede/vendor/assimp/contrib/openddlparser/code/OpenDDLParser.cpp",
		"Ganymede/vendor/assimp/contrib/openddlparser/include/openddlparser/OpenDDLParser.h",
		"Ganymede/vendor/assimp/contrib/openddlparser/include/openddlparser/OpenDDLParserUtils.h",
		"Ganymede/vendor/assimp/contrib/openddlparser/code/OpenDDLStream.cpp",
		"Ganymede/vendor/assimp/contrib/openddlparser/include/openddlparser/OpenDDLStream.h",
		"Ganymede/vendor/assimp/contrib/openddlparser/code/Value.cpp",
		"Ganymede/vendor/assimp/contrib/openddlparser/include/openddlparser/Value.h",
		"Ganymede/vendor/assimp/contrib/poly2tri/poly2tri/sweep/advancing_front.cc",
		"Ganymede/vendor/assimp/contrib/poly2tri/poly2tri/sweep/advancing_front.h",
		"Ganymede/vendor/assimp/contrib/poly2tri/poly2tri/sweep/cdt.cc",
		"Ganymede/vendor/assimp/contrib/poly2tri/poly2tri/sweep/cdt.h",
		"Ganymede/vendor/assimp/contrib/poly2tri/poly2tri/common/shapes.cc",
		"Ganymede/vendor/assimp/contrib/poly2tri/poly2tri/common/shapes.h",
		"Ganymede/vendor/assimp/contrib/poly2tri/poly2tri/sweep/sweep.cc",
		"Ganymede/vendor/assimp/contrib/poly2tri/poly2tri/sweep/sweep.h",
		"Ganymede/vendor/assimp/contrib/poly2tri/poly2tri/sweep/sweep_context.cc",
		"Ganymede/vendor/assimp/contrib/poly2tri/poly2tri/sweep/sweep_context.h",
		"Ganymede/vendor/assimp/contrib/poly2tri/poly2tri/common/utils.h",
		"Ganymede/vendor/assimp/contrib/pugixml/src/pugiconfig.hpp",
		"Ganymede/vendor/assimp/contrib/pugixml/src/pugixml.hpp",
		"Ganymede/vendor/assimp/contrib/stb/stb_image.h",
		"Ganymede/vendor/assimp/code/CApi/CInterfaceIOWrapper.cpp",
		"Ganymede/vendor/assimp/code/CApi/CInterfaceIOWrapper.h",
		"Ganymede/vendor/assimp/contrib/unzip/*",
		"Ganymede/vendor/assimp/include/assimp/Compiler/*",
		"Ganymede/vendor/assimp/code/Common/*",
		"Ganymede/vendor/assimp/contrib/zip/src/*",
		"Ganymede/vendor/assimp/code/AssetLib/STEPParser/*",
		"Ganymede/vendor/assimp/code/PostProcessing/*",
		"Ganymede/vendor/assimp/code/Material/*",
		"Ganymede/vendor/assimp/include/*",
		"Ganymede/vendor/assimp/code/Geometry/*",
		"Ganymede/vendor/assimp/contrib/clipper/*",
		"Ganymede/vendor/assimp/contrib/Open3DGC/*",
		"Ganymede/vendor/assimp/code/AssetLib/Unreal/*",
		"Ganymede/vendor/assimp/code/AssetLib/3DS/*",
		"Ganymede/vendor/assimp/code/AssetLib/3MF/*",
		"Ganymede/vendor/assimp/code/AssetLib/AC/*",
		"Ganymede/vendor/assimp/code/AssetLib/AMF/*",
		"Ganymede/vendor/assimp/code/AssetLib/ASE/*",
		"Ganymede/vendor/assimp/code/AssetLib/Assbin/*",
		"Ganymede/vendor/assimp/code/AssetLib/Assjson/*",
		"Ganymede/vendor/assimp/code/AssetLib/Assxml/*",
		"Ganymede/vendor/assimp/code/AssetLib/B3D/*",
		"Ganymede/vendor/assimp/code/AssetLib/Blender/*",
		"Ganymede/vendor/assimp/code/AssetLib/BVH/*",
		"Ganymede/vendor/assimp/code/AssetLib/COB/*",
		"Ganymede/vendor/assimp/code/AssetLib/Collada/*",
		"Ganymede/vendor/assimp/code/AssetLib/CSM/*",
		"Ganymede/vendor/assimp/code/AssetLib/DXF/*",
		"Ganymede/vendor/assimp/code/AssetLib/FBX/*",
		"Ganymede/vendor/assimp/code/AssetLib/glTF/*",
		"Ganymede/vendor/assimp/code/AssetLib/glTF2/*",
		"Ganymede/vendor/assimp/code/AssetLib/HMP/*",
		"Ganymede/vendor/assimp/code/AssetLib/IFC/IFCBoolean.cpp",
		"Ganymede/vendor/assimp/code/AssetLib/IFC/IFCCurve.cpp",
		"Ganymede/vendor/assimp/code/AssetLib/IFC/IFCGeometry.cpp",
		"Ganymede/vendor/assimp/code/AssetLib/IFC/IFCLoader.cpp",
		"Ganymede/vendor/assimp/code/AssetLib/IFC/IFCLoader.h",
		"Ganymede/vendor/assimp/code/AssetLib/IFC/IFCMaterial.cpp",
		"Ganymede/vendor/assimp/code/AssetLib/IFC/IFCOpenings.cpp",
		"Ganymede/vendor/assimp/code/AssetLib/IFC/IFCProfile.cpp",
		"Ganymede/vendor/assimp/code/AssetLib/IFC/IFCReaderGen_2x3.h",
		"Ganymede/vendor/assimp/code/AssetLib/IFC/IFCReaderGen1_2x3.cpp",
		"Ganymede/vendor/assimp/code/AssetLib/IFC/IFCReaderGen2_2x3.cpp",
		"Ganymede/vendor/assimp/code/AssetLib/IFC/IFCUtil.cpp",
		"Ganymede/vendor/assimp/code/AssetLib/IFC/IFCUtil.h",
		"Ganymede/vendor/assimp/code/AssetLib/IQM/*",
		"Ganymede/vendor/assimp/code/AssetLib/Irr/*",
		"Ganymede/vendor/assimp/code/AssetLib/LWO/*",
		"Ganymede/vendor/assimp/code/AssetLib/LWS/*",
		"Ganymede/vendor/assimp/code/AssetLib/M3D/*",
		"Ganymede/vendor/assimp/code/AssetLib/MD2/*",
		"Ganymede/vendor/assimp/code/AssetLib/MD3/*",
		"Ganymede/vendor/assimp/code/AssetLib/MD5/*",
		"Ganymede/vendor/assimp/code/AssetLib/MDC/*",
		"Ganymede/vendor/assimp/code/AssetLib/MDL/**",
		"Ganymede/vendor/assimp/code/AssetLib/MMD/*",
		"Ganymede/vendor/assimp/code/AssetLib/MS3D/*",
		"Ganymede/vendor/assimp/code/AssetLib/NDO/*",
		"Ganymede/vendor/assimp/code/AssetLib/NFF/*",
		"Ganymede/vendor/assimp/code/AssetLib/Obj/*",
		"Ganymede/vendor/assimp/code/AssetLib/OFF/*",
		"Ganymede/vendor/assimp/code/AssetLib/Ogre/*",
		"Ganymede/vendor/assimp/code/AssetLib/OpenGEX/*",
		"Ganymede/vendor/assimp/code/Pbrt/*",
		"Ganymede/vendor/assimp/code/AssetLib/Ply/*",
		"Ganymede/vendor/assimp/code/AssetLib/Q3BSP/*",
		"Ganymede/vendor/assimp/code/AssetLib/Q3D/*",
		"Ganymede/vendor/assimp/code/AssetLib/Raw/*",
		"Ganymede/vendor/assimp/code/AssetLib/SIB/*",
		"Ganymede/vendor/assimp/code/AssetLib/SMD/*",
		"Ganymede/vendor/assimp/code/AssetLib/Step/StepExporter.cpp",
		"Ganymede/vendor/assimp/code/AssetLib/Step/StepExporter.h",
		"Ganymede/vendor/assimp/code/AssetLib/STL/*",
		"Ganymede/vendor/assimp/code/AssetLib/Terragen/*",
		"Ganymede/vendor/assimp/code/AssetLib/X/*",
		"Ganymede/vendor/assimp/code/AssetLib/X3D/*",
		"Ganymede/vendor/assimp/code/AssetLib/XGL/*"
	}

	includedirs
	{
		"Ganymede/vendor/assimp/build/include",
		"Ganymede/vendor/assimp/build",
		"Ganymede/vendor/assimp/include",
		"Ganymede/vendor/assimp/code",
		"Ganymede/vendor/assimp/.",
		"Ganymede/vendor/assimp/contrib/zlib",
		"Ganymede/vendor/assimp/build/contrib/zlib",
		"Ganymede/vendor/assimp/code/../contrib/pugixml/src",
		"Ganymede/vendor/assimp/code/../contrib/utf8cpp/source",
		"Ganymede/vendor/assimp/code/../contrib/rapidjson/include",
		"Ganymede/vendor/assimp/code/../contrib",
		"Ganymede/vendor/assimp/code/../contrib/unzip",
		"Ganymede/vendor/assimp/code/../contrib/openddlparser/include",
		"Ganymede/vendor/assimp/code/../include",
		"Ganymede/vendor/assimp/build/code/../include",
		"Ganymede/vendor/assimp/code/.."
	}

	links
	{
		"zlib"
	}
	
	flags { "MultiProcessorCompile" }

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

	filter "configurations:Debug"
		defines "GM_DEBUG"
		symbols "On"
		defines
		{
			"WIN32",
			"_WINDOWS",
			"_DEBUG",
			"OPENDDL_STATIC_LIBARY",
			"P2T_STATIC_EXPORTS",
			"ASSIMP_BUILD_NO_M3D_IMPORTER",
			"ASSIMP_BUILD_NO_M3D_EXPORTER",
			"WIN32_LEAN_AND_MEAN",
			"UNICODE",
			"_UNICODE",
			"ASSIMP_BUILD_NO_C4D_IMPORTER",
			"ASSIMP_BUILD_NO_USD_IMPORTER",
			"MINIZ_USE_UNALIGNED_LOADS_AND_STORES=0",
			"ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC=1",
			"RAPIDJSON_HAS_STDSTRING=1",
			"RAPIDJSON_NOMEMBERITERATORCLASS",
			"_SCL_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_WARNINGS",
			"OPENDDLPARSER_BUILD",
			"CMAKE_INTDIR='Debug'"
		}
		
	filter "configurations:Release"
		defines "GM_RELEASE"
		optimize "On"
		defines
		{
			"WIN32",
			"_WINDOWS",
			"NDEBUG",
			"OPENDDL_STATIC_LIBARY",
			"P2T_STATIC_EXPORTS",
			"ASSIMP_BUILD_NO_M3D_IMPORTER",
			"ASSIMP_BUILD_NO_M3D_EXPORTER",
			"WIN32_LEAN_AND_MEAN",
			"UNICODE",
			"_UNICODE",
			"ASSIMP_BUILD_NO_C4D_IMPORTER",
			"ASSIMP_BUILD_NO_USD_IMPORTER",
			"MINIZ_USE_UNALIGNED_LOADS_AND_STORES=0",
			"ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC=1",
			"RAPIDJSON_HAS_STDSTRING=1",
			"RAPIDJSON_NOMEMBERITERATORCLASS",
			"_SCL_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_WARNINGS",
			"OPENDDLPARSER_BUILD",
			"CMAKE_INTDIR='Release'"
		}

	filter "configurations:Retail"
		defines "GM_RETAIL"
		optimize "On"
		defines
		{
			"WIN32",
			"_WINDOWS",
			"NDEBUG",
			"OPENDDL_STATIC_LIBARY",
			"P2T_STATIC_EXPORTS",
			"ASSIMP_BUILD_NO_M3D_IMPORTER",
			"ASSIMP_BUILD_NO_M3D_EXPORTER",
			"WIN32_LEAN_AND_MEAN",
			"UNICODE",
			"_UNICODE",
			"ASSIMP_BUILD_NO_C4D_IMPORTER",
			"ASSIMP_BUILD_NO_USD_IMPORTER",
			"MINIZ_USE_UNALIGNED_LOADS_AND_STORES=0",
			"ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC=1",
			"RAPIDJSON_HAS_STDSTRING=1",
			"RAPIDJSON_NOMEMBERITERATORCLASS",
			"_SCL_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_WARNINGS",
			"OPENDDLPARSER_BUILD",
			"CMAKE_INTDIR='Release'"
		}


project "zlib"
	location "Ganymede/vendor/assimp"
	kind "StaticLib"
	language "C++"

	targetdir (bindir .. outputdir .. "/%{prj.name}")
	objdir (intermediatedir .. outputdir .. "/%{prj.name}")

	files
	{
		"Ganymede/vendor/assimp/contrib/zlib/*.h",
		"Ganymede/vendor/assimp/contrib/zlib/*.c",
		"Ganymede/vendor/assimp/build/contrib/zlib/zconf.h"
	}

	includedirs
	{
		"Ganymede/vendor/assimp/build/include",
		"Ganymede/vendor/assimp/build",
		"Ganymede/vendor/assimp/include",
		"Ganymede/vendor/assimp/code",
		"Ganymede/vendor/assimp/.",
		"Ganymede/vendor/assimp/contrib/zlib",
		"Ganymede/vendor/assimp/build/contrib/zlib",
		"Ganymede/vendor/assimp"
	}
	
	flags { "MultiProcessorCompile" }

	filter "system:windows"
		cppdialect "C++14"
		staticruntime "On"
		systemversion "latest"

	filter "configurations:Debug"
		defines "GM_DEBUG"
		symbols "On"
		defines
		{
			"WIN32",
			"_WINDOWS",
			"ASSIMP_BUILD_NO_M3D_IMPORTER",
			"ASSIMP_BUILD_NO_M3D_EXPORTER",
			"WIN32_LEAN_AND_MEAN",
			"UNICODE",
			"_UNICODE",
			"NO_FSEEKO",
			"_CRT_SECURE_NO_DEPRECATE",
			"_CRT_NONSTDC_NO_DEPRECATE",
			"CMAKE_INTDIR='Debug'"
		}
		
	filter "configurations:Release"
		defines "GM_RELEASE"
		optimize "On"
		defines
		{
			"WIN32",
			"_WINDOWS",
			"NDEBUG",
			"ASSIMP_BUILD_NO_M3D_IMPORTER",
			"ASSIMP_BUILD_NO_M3D_EXPORTER",
			"WIN32_LEAN_AND_MEAN",
			"UNICODE",
			"_UNICODE",
			"NO_FSEEKO",
			"_CRT_SECURE_NO_DEPRECATE",
			"_CRT_NONSTDC_NO_DEPRECATE",
			"CMAKE_INTDIR='Release'"
		}

	filter "configurations:Retail"
		defines "GM_RETAIL"
		optimize "On"
		defines
		{
			"WIN32",
			"_WINDOWS",
			"NDEBUG",
			"ASSIMP_BUILD_NO_M3D_IMPORTER",
			"ASSIMP_BUILD_NO_M3D_EXPORTER",
			"WIN32_LEAN_AND_MEAN",
			"UNICODE",
			"_UNICODE",
			"NO_FSEEKO",
			"_CRT_SECURE_NO_DEPRECATE",
			"_CRT_NONSTDC_NO_DEPRECATE",
			"CMAKE_INTDIR='Release'"
		}