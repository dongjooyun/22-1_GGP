#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"
#include "Renderer/DataTypes.h"
#include "Renderer/Renderable.h"

class MyCube : public BaseCube
{
public:
	MyCube() = default;
	~MyCube() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;
};