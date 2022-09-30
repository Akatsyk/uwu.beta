#pragma once

#include "../sdk/sdk.hpp"
#include "../hooks.hpp"

class Chams : public Singleton <Chams>
{

	friend class Singleton<Chams>;

public:

	Chams();
	~Chams();

	IMaterial* material_default = nullptr;
	IMaterial* material_flat = nullptr;
	IMaterial* material_glow = nullptr;
	IMaterial* material_metallic = nullptr;
	IMaterial* material_circuit = nullptr;

	struct
	{
		IMaterial* material_1 = nullptr;
		IMaterial* material_2 = nullptr;
		IMaterial* material_3 = nullptr;
		IMaterial* material_4 = nullptr;
	} Smoke_Materials;

	void OverrideMaterial(bool ignoreZ, bool use_env, const Color& rgba, IMaterial* material, float alpha);

	void OnDrawModelExecute(
		IMatRenderContext* ctx,
		const DrawModelState_t& state,
		const ModelRenderInfo_t& pInfo,
		matrix3x4_t* pCustomBoneToWorld);
};