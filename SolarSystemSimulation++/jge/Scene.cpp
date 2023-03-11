#include <algorithm>    // std::sort

#include "Scene.h"
#include "Camera.h"
#include "Model.h"
#include "TransparencySorter.h"
#include "Measurement.h"

#include <glm/glm.hpp>

using namespace glm;

namespace jge
{
    Scene::Scene()
        : enableSorting(true)
        , lodLevel1Distance(30.0f)
        , lodLevel2Distance(60.0f)
        , camera(nullptr)
        , skyBoxTexture(0)
    {
    }


    Scene::~Scene()
    {
    }

    void Scene::AddLight(LightSource* light)
    {
        if (lights.size() < MAX_LIGHTS)
        {
            lights.push_back(light);
        }
    }

    void Scene::AddModel(Model* model)
    {
        if (model->IsTransparent())
        {
            transparentModels.push_back(model);

            if (model->IsSortingTrianglesRequired())
                sortedModels.push_back(model);
        }
        else opaqueModels.push_back(model);

        if (model->IsCastingShadow())
            shadowCastingModels.push_back(model);

        if (model->IsGlowing())
            glowingModels.push_back(model);
        else if (model->IsCastingShadow())
        {
            // If the model is not shadowcasting, there is no
            // reason it should occlude glowing objects
            nonglowingModels.push_back(model);
        }
    }

    void Scene::RemoveModel(Model* model)
    {
        transparentModels.erase(
            std::remove(transparentModels.begin(), transparentModels.end(), model), transparentModels.end());
        opaqueModels.erase(
            std::remove(opaqueModels.begin(), opaqueModels.end(), model), opaqueModels.end());
        shadowCastingModels.erase(
            std::remove(shadowCastingModels.begin(), shadowCastingModels.end(), model), shadowCastingModels.end());
        glowingModels.erase(
            std::remove(glowingModels.begin(), glowingModels.end(), model), glowingModels.end());
        nonglowingModels.erase(
            std::remove(nonglowingModels.begin(), nonglowingModels.end(), model), nonglowingModels.end());
    }

    void Scene::SetCamera(Camera* c)
    {
        camera = c;
    }

    void Scene::SetSkyboxTexture(GLuint tex)
    {
        skyBoxTexture = tex;
    }

    void Scene::EnableTriangleSorting(bool state)
    {
        enableSorting = state;
    }

    bool Scene::IsTriangleSortingEnabled() const
    {
        return enableSorting;
    }

    int Scene::DetermineLODLevel(Model& m) const
    {
        if (m.DistanceToCamera() > lodLevel2Distance)
            return 2;
        if (m.DistanceToCamera() > lodLevel1Distance)
            return 1;
        else return 0;
    }

    struct ModelDistanceComparer
    {
        Camera* camera;
        ModelDistanceComparer(Camera* c)
        {
            camera = c;
        }

        inline bool operator() (Model* m1, Model* m2)
        {
            m1->DistanceToCamera() = glm::length(camera->GetPosition() - m1->GetPosition());
            m2->DistanceToCamera() = glm::length(camera->GetPosition() - m2->GetPosition());

            return (m1->DistanceToCamera() > m2->DistanceToCamera());
        }
    };

    struct ShaderComparer
    {
        inline bool operator() (Model* m1, Model* m2)
        {
            return (m1->GetShader() > m2->GetShader());
        }
    };

    void Scene::SortModelsByDistance()
    {
        std::sort(transparentModels.begin(), transparentModels.end(), ModelDistanceComparer(camera));
        std::sort(opaqueModels.begin(), opaqueModels.end(), ModelDistanceComparer(camera));

        std::stable_sort(transparentModels.begin(), transparentModels.end(), ShaderComparer());
        std::stable_sort(opaqueModels.begin(), opaqueModels.end(), ShaderComparer());
    }

    void Scene::SortTransparentTrianglesByDistance()
    {
        for (unsigned int i = 0; i < sortedModels.size(); ++i)
        {
            int lodLevel = DetermineLODLevel(*sortedModels[i]);
            Mesh* lvlMesh = sortedModels[i]->GetLODMesh(lodLevel);			// Instanciated....! #FIX!

            TransparencySorter::SortTrianglesByDistanceToCamera(camera->GetPosition(), lvlMesh, sortedModels[i]->modelMatrix);

            lvlMesh->Update();
        }
    }

    void Scene::Optimize()
    {
        SortModelsByDistance();

        if (enableSorting)
            SortTransparentTrianglesByDistance();
    }
}