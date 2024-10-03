#pragma once
#include <vector>

class PointlightWorldObjectInstance;

class LightsManager
{
public:
	enum LightingState
	{
		Initialize,
		DynamicShadow,
		StaticShadow,
		NoShadow
	};

	static unsigned int MAX_POINTLIGHTS_STATICS_SHADOWS;
	static unsigned int MAX_POINTLIGHTS_DYNAMIC_SHADOWS;

	LightsManager();
	~LightsManager() {};

	void Update(const std::vector<PointlightWorldObjectInstance*>& pointlightsSortedByDistanceToCamera);

private:
	std::vector<int> m_LightIDStorage;
};