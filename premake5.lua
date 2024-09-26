workspace "Ganymede"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Retail"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Ganymede"
	location "Ganymede"
	kind "SharedLib"
	language "C++"

	targetdir (".bin/" .. outputdir .. "/%{prj.name}")
	objdir (".intermediate/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"GM_PLATFORM_WINDOWS",
			"GM_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPYFILE} %{cfg.buildtarget.relpath} ../.bin/" .. outputdir .. "/GanymedeApp")
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

	targetdir (".bin/" .. outputdir .. "/%{prj.name}")
	objdir (".intermediate/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Ganymede/src"
	}

	links
	{
		"Ganymede"
	}

	filter "system:windows"
		cppdialect "C++17"
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