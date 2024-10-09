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


group "Dependecies/CMAKE"

externalproject "assimp"
   location "Ganymede/vendor/assimp/build/code"
   uuid "11111111-1111-1111-1111-111111111111"
   kind "None"
   language "C++"

externalproject "glfw"
   location "Ganymede/vendor/glfw/build/src"
   uuid "22222222-2222-2222-2222-222222222222"
   kind "None"
   language "C++"

externalproject "glm"
   location "Ganymede/vendor/glm/build/glm"
   uuid "33333333-3333-3333-3333-333333333333"
   kind "None"
   language "C++"

externalproject "glew_s"
   location "Ganymede/vendor/glew/build/cmake/build"
   uuid "44444444-4444-4444-4444-444444444444"
   kind "None"
   language "C"

group ""

group "Dependecies"

project "DetourRecast"
        location "Ganymede/vendor/recastnavigation"
        kind "StaticLib"
        language "C++"
	staticruntime "Off"

        targetdir (bindir .. outputdir .. "/%{prj.name}")
	objdir (intermediatedir .. outputdir .. "/%{prj.name}")

	files
	{
		"Ganymede/vendor/recastnavigation/Detour/include/**.h",
		"Ganymede/vendor/recastnavigation/Detour/source/**.cpp",
		"Ganymede/vendor/recastnavigation/DetourCrowd/include/**.h",
		"Ganymede/vendor/recastnavigation/DetourCrowd/source/**.cpp",
		"Ganymede/vendor/recastnavigation/DetourTileCache/include/**.h",
		"Ganymede/vendor/recastnavigation/DetourTileCache/source/**.cpp",
		"Ganymede/vendor/recastnavigation/Recast/include/**.h",
		"Ganymede/vendor/recastnavigation/Recast/source/**.cpp"
	}

	includedirs
	{
		"Ganymede/vendor/recastnavigation/Detour/include",
		"Ganymede/vendor/recastnavigation/DetourCrowd/include",
		"Ganymede/vendor/recastnavigation/DetourTileCache/include",
		"Ganymede/vendor/recastnavigation/Recast/include"
	}

        filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

	filter "configurations:Debug"
		symbols "On"

	filter { "configurations:Release or configurations:Retail" }
    		optimize "On"


project "stb_image"
	location "Ganymede/vendor/stb_image"
	kind "StaticLib"
	language "C"
	staticruntime "Off"

	targetdir (bindir .. outputdir .. "/%{prj.name}")
	objdir (intermediatedir .. outputdir .. "/%{prj.name}")

	files
	{
		"Ganymede/vendor/stb_image/**.h",
		"Ganymede/vendor/stb_image/**.cpp"
	}

	includedirs
	{
		"Ganymede/vendor/stb_image"
	}


	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		symbols "On"

	filter { "configurations:Release or configurations:Retail" }
    		optimize "On"

group ""


project "Ganymede"
	location "Ganymede"
	kind "SharedLib"
	language "C++"
	staticruntime "Off"

	flags { "MultiProcessorCompile" }

	targetdir (bindir .. outputdir .. "/%{prj.name}")
	objdir (intermediatedir .. outputdir .. "/%{prj.name}")

	defines "GLM_ENABLE_EXPERIMENTAL"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Ganymede/src",
		"Ganymede/vendor/spdlog/include",
		"Ganymede/vendor/assimp/include",
		"Ganymede/vendor/assimp/build/include",
		"Ganymede/vendor/glm",
		"Ganymede/vendor/glfw/include",
		"Ganymede/vendor/bullet3/src",
		"Ganymede/vendor/glew/include",
		"Ganymede/vendor/recastnavigation/Detour/include",
		"Ganymede/vendor/recastnavigation/DetourCrowd/include",
		"Ganymede/vendor/recastnavigation/DetourTileCache/include",
		"Ganymede/vendor/recastnavigation/Recast/include",
		"Ganymede/vendor/stb_image"
	}

	filter "system:windows"
		cppdialect "C++20"
		systemversion "latest"

		defines
		{
			"GM_PLATFORM_WINDOWS",
			"GM_BUILD_DLL",
			"GLEW_STATIC"
		}

		postbuildcommands
		{
 			("{MKDIR} ../" .. bindir .. outputdir .. "/GanymedeApp"),
			("{COPYFILE} %{cfg.buildtarget.relpath} ../" .. bindir .. outputdir .. "/GanymedeApp")
		}

	filter "configurations:Debug"
		defines "GM_DEBUG"
		symbols "On"
		libdirs
		{
			"Ganymede/vendor/assimp/build/lib/Debug",
			"Ganymede/vendor/assimp/build/contrib/zlib/Debug",
			"Ganymede/vendor/glfw/build/src/Debug",
			"Ganymede/vendor/glm/build/glm/Debug",
			"Ganymede/vendor/glew/build/cmake/build/lib/Debug"
		}
		links
		{
			"stb_image",
			"DetourRecast",
			"opengl32.lib",
			"libglew32d.lib",
			"zlibstaticd.lib",
			"assimp-vc143-mtd.lib",
			"glfw3.lib",
			"glm.lib",
			"Ganymede/vendor/bullet3/build/lib/Debug/*.lib"
		}
		prebuildcommands
    		{
        		"call \"$(SolutionDir)buildscripts/BuildAssimp.bat\" Debug",
			"call \"$(SolutionDir)buildscripts/BuildGlfw.bat\" Debug",
			"call \"$(SolutionDir)buildscripts/BuildGLM.bat\" Debug",
			"call \"$(SolutionDir)buildscripts/BuildBullet3.bat\" Debug",
			"call \"$(SolutionDir)buildscripts/BuildGlew.bat\" Debug"
    		}

	filter { "configurations:Release or configurations:Retail" }
    		optimize "On"
    		libdirs
    		{	
        		"Ganymede/vendor/assimp/build/lib/Release",
        		"Ganymede/vendor/assimp/build/contrib/zlib/Release",
			"Ganymede/vendor/glfw/build/src/Release",
			"Ganymede/vendor/glm/build/glm/Release",
			"Ganymede/vendor/glew/build/cmake/build/lib/Release"
    		}
    		links
    		{
			"stb_image",
			"DetourRecast",
			"opengl32.lib",
			"libglew32.lib",
       			"zlibstatic.lib",
        		"assimp-vc143-mt.lib",
			"glfw3.lib",
			"glm.lib",
			"Ganymede/vendor/bullet3/build/lib/Release/*.lib"
    		}
		prebuildcommands
    		{
        		"call \"$(SolutionDir)buildscripts/BuildAssimp.bat\" Release",
			"call \"$(SolutionDir)buildscripts/BuildGlfw.bat\" Release",
			"call \"$(SolutionDir)buildscripts/BuildGLM.bat\" Release",
			"call \"$(SolutionDir)buildscripts/BuildBullet3.bat\" Release",
			"call \"$(SolutionDir)buildscripts/BuildGlew.bat\" Release"
    		}

	filter "configurations:Release"
    		defines "GM_RELEASE"

	filter "configurations:Retail"
    		defines "GM_RETAIL"


project "GanymedeApp"
	location "GanymedeApp"
	kind "ConsoleApp"
	language "C++"
	staticruntime "Off"

	flags { "MultiProcessorCompile" }

	defines "GLM_ENABLE_EXPERIMENTAL"

	targetdir (bindir .. outputdir .. "/%{prj.name}")
	objdir (intermediatedir .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/res/**"
	}

	includedirs
	{
		"Ganymede/src",
		"Ganymede/vendor/spdlog/include",
		"Ganymede/vendor/glm"
	}
	
	libdirs
    	{	
		"Ganymede/vendor/glew/build/cmake/build/lib/Release"
    	}
	links
	{
		"Ganymede"
	}

	filter "system:windows"
		cppdialect "C++20"
		systemversion "latest"

		defines
		{
			"GM_PLATFORM_WINDOWS",
			"GLEW_STATIC"
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