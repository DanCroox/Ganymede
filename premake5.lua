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
		"Ganymede/vendor/spdlog/include"
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
		"Ganymede"
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