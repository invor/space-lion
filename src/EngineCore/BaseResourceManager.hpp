/// <copyright file="BaseResourceManager.hpp">
/// Copyright © 2018 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef BaseResourceManager_hpp
#define BaseResourceManager_hpp

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "MTQueue.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        const size_t MAX_RESOURCE_ID = (std::numeric_limits<unsigned int>::max)() - 1;
        /**
        * Most basic (GPU) resource identifier using only a unique integer id.
        * The id of a resource can't be changed after construction
        * and only the ResourceManager is allowed to construct new ResourceIDs.
        */
        struct ResourceID
        {
            inline unsigned int value() const { return m_id; }

            inline bool operator==(const ResourceID& rhs) const { return m_id == rhs.value(); }
            inline bool operator!=(const ResourceID& rhs) const { return m_id != rhs.value(); }

            template<typename Buffer, typename Mesh, typename ShaderProgram, typename Texture2D, typename Texture3D>
            friend class BaseResourceManager;
            template<typename ResourceType>
            friend struct WeakResource;
        private:
            ResourceID() : m_id((std::numeric_limits<unsigned int>::max)()) {}
            ResourceID(unsigned int id) : m_id(id) {}
            unsigned int m_id;
        };

        enum ResourceState { NOT_READY, READY, EXPIRED };

        /** Non-owning resource struct for more direct reference to resources */
        template<typename ResourceType>
        struct WeakResource
        {
            //WeakResource() : id(BaseResourceManager::invalidResourceID()), resource(nullptr), state(NOT_READY) {}
            WeakResource() : id(ResourceID()), resource(nullptr), state(NOT_READY) {}
            //WeakResource() = delete;
            WeakResource(ResourceID id, ResourceType* resource, ResourceState state) : id(id), resource(resource), state(state) {}

            ResourceID		id;
            ResourceType*	resource;
            ResourceState	state;
        };

        /** Owning resource struct for internal storage of resources */
        template<typename ResourceType>
        struct Resource
        {
            //Resource() : id(GEngineCore::resourceManager().getInvalidResourceID()), resource(nullptr), state(NOT_READY) {}
            Resource(ResourceID id) : id(id), resource(nullptr), state(NOT_READY) {}

            ResourceID						id;
            std::unique_ptr<ResourceType>	resource;
            ResourceState					state;
        };

        template<
            typename Buffer,
            typename Mesh,
            typename ShaderProgram,
            typename Texture2D,
            typename Texture3D>
            class BaseResourceManager
        {
        public:
            BaseResourceManager() = default;
            BaseResourceManager(BaseResourceManager const & cpy) = delete;
            ~BaseResourceManager() = default;

            static ResourceID invalidResourceID() {
                return ResourceID();
            }

            virtual void clearAllResources() = 0;

            void executeRenderThreadTasks() {
                while (!m_renderThread_tasks.empty())
                {
                    m_renderThread_tasks.pop()();
                }
            }

            WeakResource<Buffer> getBufferResource(ResourceID rsrc_id);

            WeakResource<Buffer> getBufferResource(std::string const& rsrc_name);

            WeakResource<Mesh> getMeshResource(ResourceID rsrc_id);

            WeakResource<ShaderProgram> getShaderProgramResource(ResourceID rsrc_id);

            WeakResource<Texture2D> getTexture2DResource(ResourceID rsrc_id);

            WeakResource<Texture2D> getTexture2DResource(std::string name);

        protected:

            inline std::pair<bool, size_t> getIndex(ResourceID rsrc_id, std::unordered_map<unsigned int, size_t> const& index_lut)
            {
                std::pair<bool, size_t> retval = { false,0 };

                auto query = index_lut.find(rsrc_id.value());

                if (query != m_id_to_mesh_idx.end())
                {
                    retval = { true,query->second };
                }

                return retval;
            }

            inline void addMeshIndex(unsigned int rsrc_id, std::string name, size_t index) {
                m_id_to_mesh_idx.insert(std::pair<unsigned int, size_t>(rsrc_id, index));
                m_name_to_mesh_idx.insert(std::pair<std::string, size_t>(name, index));
            }

            inline void addTextureIndex(unsigned int rsrc_id, std::string name, size_t index) {
                m_id_to_textures_2d_idx.insert(std::pair<unsigned int, size_t>(rsrc_id, index));
                m_name_to_textures_2d_idx.insert(std::pair<std::string, size_t>(name, index));
            }

            ResourceID generateResourceID() {
                std::unique_lock<std::mutex> rsrcID_lock(m_rsrcID_mutex);
                return ResourceID(m_resource_cnt++);
            };

            /** Total number of created resources. Used to create ResourceIDs. */
            unsigned int m_resource_cnt;

            /** Mutex to protect ResourceID generation. */
            mutable std::mutex m_rsrcID_mutex;

            /** Queue for exection of stuff on the render thread. */
            Utility::MTQueue<std::function<void()>> m_renderThread_tasks;

            /*
             * The following collections contain all resources that are managed by an instance of a resource manager.
             * There is only a single "instance" of any (uniquely identifiable) resouce kept/referenced in these collections.
             * Different entities making use of the same resource will both be refering to the single
             * instance kept/referenced within one of these collections.
             * Resources are kept by a unique pointer for exclusive resource ownership and are paired with an ID by
             * which they are referenced outside of resource management and rendering.
             */
            std::vector<Resource<Buffer>>        m_buffers;
            std::vector<Resource<Mesh>>          m_meshes;
            std::vector<Resource<ShaderProgram>> m_shader_programs;
            std::vector<Resource<Texture2D>>     m_textures_2d;
            std::vector<Resource<Texture3D>>     m_textures_3d;

            std::vector<std::string>             m_shader_programs_identifier;

            std::unordered_map<unsigned int, size_t> m_id_to_buffer_idx;
            std::unordered_map<unsigned int, size_t> m_id_to_mesh_idx;
            std::unordered_map<unsigned int, size_t> m_id_to_shader_program_idx;
            std::unordered_map<unsigned int, size_t> m_id_to_textures_2d_idx;
            std::unordered_map<unsigned int, size_t> m_id_to_textures_3d_idx;

            std::unordered_map<std::string, size_t> m_name_to_buffer_idx;
            std::unordered_map<std::string, size_t> m_name_to_mesh_idx;
            std::unordered_map<std::string, size_t> m_name_to_shader_program_idx;
            std::unordered_map<std::string, size_t> m_name_to_textures_2d_idx;
            std::unordered_map<std::string, size_t> m_name_to_textures_3d_idx;

            mutable std::shared_mutex m_buffers_mutex;
            mutable std::shared_mutex m_meshes_mutex;
            mutable std::shared_mutex m_shader_programs_mutex;
            mutable std::shared_mutex m_textures_2d_mutex;
            mutable std::shared_mutex m_textures_3d_mutex;
        };

        template<typename Buffer, typename Mesh, typename ShaderProgram, typename Texture2D, typename Texture3D>
        inline WeakResource<Buffer> BaseResourceManager<Buffer, Mesh, ShaderProgram, Texture2D, Texture3D>::getBufferResource(ResourceID rsrc_id)
        {
            std::shared_lock<std::shared_mutex> lock(m_buffers_mutex);

            WeakResource<Mesh> retval(rsrc_id, nullptr, NOT_READY);

            auto query = m_id_to_buffer_idx.find(rsrc_id.value());

            if (query != m_id_to_buffer_idx.end())
            {
                retval = WeakResource<Buffer>(
                    m_meshes[query->second].id,
                    m_meshes[query->second].resource.get(),
                    m_meshes[query->second].state);
            }

            return retval;
        }

        template<typename Buffer, typename Mesh, typename ShaderProgram, typename Texture2D, typename Texture3D>
        inline WeakResource<Buffer> BaseResourceManager<Buffer, Mesh, ShaderProgram, Texture2D, Texture3D>::getBufferResource(std::string const& rsrc_name)
        {
            WeakResource<Buffer> retval;

            std::shared_lock<std::shared_mutex> lock(m_buffers_mutex);
            auto search = m_name_to_buffer_idx.find(rsrc_name);
            if (search != m_name_to_buffer_idx.end())
            {
                retval = WeakResource<Buffer>(
                    m_buffers[search->second].id,
                    m_buffers[search->second].resource.get(),
                    m_buffers[search->second].state);
            }
             
            return retval;
        }

        template<typename Buffer, typename Mesh, typename ShaderProgram, typename Texture2D, typename Texture3D>
        inline WeakResource<Mesh> BaseResourceManager<Buffer, Mesh, ShaderProgram, Texture2D, Texture3D>::getMeshResource(ResourceID rsrc_id)
        {
            std::shared_lock<std::shared_mutex> lock(m_meshes_mutex);

            WeakResource<Mesh> retval(rsrc_id, nullptr, NOT_READY);

            auto query = m_id_to_mesh_idx.find(rsrc_id.value());

            if (query != m_id_to_mesh_idx.end())
            {
                retval = WeakResource<Mesh>(
                    m_meshes[query->second].id,
                    m_meshes[query->second].resource.get(),
                    m_meshes[query->second].state);
            }

            return retval;
        }

        template<typename Buffer, typename Mesh, typename ShaderProgram, typename Texture2D, typename Texture3D>
        inline WeakResource<ShaderProgram> BaseResourceManager<Buffer, Mesh, ShaderProgram, Texture2D, Texture3D>::getShaderProgramResource(ResourceID rsrc_id)
        {
            std::shared_lock<std::shared_mutex> lock(m_shader_programs_mutex);

            WeakResource<ShaderProgram> retval(rsrc_id, nullptr, NOT_READY);

            auto query = m_id_to_shader_program_idx.find(rsrc_id.value());

            if (query != m_id_to_shader_program_idx.end())
            {
                retval = WeakResource<ShaderProgram>(
                    m_shader_programs[query->second].id,
                    m_shader_programs[query->second].resource.get(),
                    m_shader_programs[query->second].state);
            }

            return retval;
        }

        template<typename Buffer, typename Mesh, typename ShaderProgram, typename Texture2D, typename Texture3D>
        inline WeakResource<Texture2D> BaseResourceManager<Buffer, Mesh, ShaderProgram, Texture2D, Texture3D>::getTexture2DResource(ResourceID rsrc_id)
        {
            std::shared_lock<std::shared_mutex> lock(m_textures_2d_mutex);

            WeakResource<Texture2D> retval(rsrc_id, nullptr, NOT_READY);

            auto query = m_id_to_textures_2d_idx.find(rsrc_id.value());

            if (query != m_id_to_textures_2d_idx.end())
            {
                retval = WeakResource<Texture2D>(
                    m_textures_2d[query->second].id,
                    m_textures_2d[query->second].resource.get(),
                    m_textures_2d[query->second].state);
            }

            return retval;
        }

        template<typename Buffer, typename Mesh, typename ShaderProgram, typename Texture2D, typename Texture3D>
        inline WeakResource<Texture2D> BaseResourceManager<Buffer, Mesh, ShaderProgram, Texture2D, Texture3D>::getTexture2DResource(std::string name)
        {
            std::shared_lock<std::shared_mutex> lock(m_textures_2d_mutex);

            WeakResource<Texture2D> retval(invalidResourceID(), nullptr, NOT_READY);

            auto query = m_name_to_textures_2d_idx.find(name);

            if (query != m_name_to_textures_2d_idx.end())
            {
                retval = WeakResource<Texture2D>(
                    m_textures_2d[query->second].id,
                    m_textures_2d[query->second].resource.get(),
                    m_textures_2d[query->second].state);
            }

            return retval;
        }
    }
}

#endif // !BaseResourceManager_hpp