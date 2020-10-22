#pragma once


#include <vector>

#include "LightSource.h"
#include "Camera.h"
#include "Model.h"
#include "Framebuffer.h"

namespace jge
{
	const int MAX_LIGHTS = 4;

	class Scene
	{
	public:
		Scene();
		~Scene();

		// Initial Setup
		void SetCamera(Camera* c);

		// Add stuff to the scene. Model will be sorted into
		// apropriate lists.
		void AddLight(LightSource* light);
		void AddModel(Model* model);
        void RemoveModel(Model*);
		void SetSkyboxTexture(GLuint tex);

		// This enables correctly rendered transparency.
		void EnableTriangleSorting(bool);
		bool IsTriangleSortingEnabled() const;

		// Sort the models by distance to allow for an early z-test
		// If enabled, sort triangles to get transparency right.
		void Optimize();

		// Allow the render pipeline to access our private members
		// like sorted models, lights and other render relevant stuff.
		friend class RenderPipeline;

	private:
		// Rendering order
		// Sort models to get transparency right (if enabled)
		void SortModelsByDistance();
		void SortTransparentTrianglesByDistance();
		
		// Some helper
		int DetermineLODLevel(Model& m) const;
		
		float lodLevel1Distance;
		float lodLevel2Distance;
		bool enableSorting;						// Is sorting of triangles enabled?

		Camera* camera;
		GLuint skyBoxTexture;

		// Added models are sorted by their properties to make rendering more efficient
		std::vector<Model*> sortedModels;		// Sorting List
		std::vector<Model*> transparentModels;	// Render List for Transparent Models
		std::vector<Model*> opaqueModels;		// Render List for Opaque Models
		std::vector<Model*> shadowCastingModels;
		std::vector<Model*> glowingModels;
		std::vector<Model*> nonglowingModels;

		std::vector<LightSource*> lights;
	};
}