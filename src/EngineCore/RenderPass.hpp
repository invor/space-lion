#ifndef RenderPass_hpp
#define RenderPass_hpp

#include <functional>

namespace EngineCore
{
    namespace Graphics
    {

        struct RenderPassConcept
        {
            virtual ~RenderPassConcept() {}
            virtual RenderPassConcept* clone() const = 0;
            virtual void setupData() = 0;
            virtual void setupResources() = 0;
            virtual void execute() const = 0;
        };

        template<typename D, typename R>
        using SetupCallback = std::function<void(D&, R&)>;
        template<typename D, typename R>
        using ExecuteCallback = std::function<void(D const&, R const&)>;

        template<typename Data, typename Resources>
        struct RenderPassModel : RenderPassConcept
        {
            RenderPassModel(
                SetupCallback<Data, Resources> data_setup,
                SetupCallback<Data, Resources> resources_setup,
                ExecuteCallback<Data, Resources> execute)
                : m_data_setup(data_setup),
                m_resources_setup(resources_setup),
                m_execute(execute) {}

            RenderPassModel(
                Data data,
                Resources resources,
                SetupCallback<Data, Resources> data_setup,
                SetupCallback<Data, Resources> resources_setup,
                ExecuteCallback<Data, Resources> execute)
                : m_data(data),
                m_resources(resources),
                m_data_setup(data_setup),
                m_resources_setup(resources_setup),
                m_execute(execute) {}

            RenderPassConcept* clone() const
            {
                return new RenderPassModel(
                    m_data,
                    m_resources,
                    m_data_setup,
                    m_resources_setup,
                    m_execute);
            }

            void setupData() { m_data_setup(m_data, m_resources); }

            void setupResources() { m_resources_setup(m_data, m_resources); }

            void execute() const { m_execute(m_data, m_resources); }

            Data		m_data;
            Resources	m_resources;

            SetupCallback<Data, Resources>		m_data_setup;
            SetupCallback<Data, Resources>		m_resources_setup;
            ExecuteCallback<Data, Resources>	m_execute;
        };

        class RenderPass
        {
        public:
            template<typename PassData, typename PassResources>
            RenderPass(
                std::string const& description,
                SetupCallback<PassData, PassResources> data_setup,
                SetupCallback<PassData, PassResources> resources_setup,
                ExecuteCallback<PassData, PassResources> execute)
                : m_description(description),
                m_pass(new RenderPassModel<PassData, PassResources>(data_setup, resources_setup, execute)) {}

            RenderPass() : m_pass(nullptr) {};

            ~RenderPass()
            {
                if (m_pass != nullptr)
                    delete m_pass;
            }

            RenderPass(RenderPass const& other) : m_description(other.m_description), m_pass(other.m_pass->clone()) {}
            RenderPass(RenderPass&& other) : RenderPass() {
                std::swap(m_description, other.m_description);
                std::swap(m_pass, other.m_pass);
            }

            RenderPass& operator=(RenderPass const& rhs)
            {
                delete m_pass;
                m_pass = rhs.m_pass->clone();

                return *this;
            }

            RenderPass& operator=(RenderPass&& other) = delete;

            void setupData() { m_pass->setupData(); }

            void setupResources() { m_pass->setupResources(); }

            void execute() { m_pass->execute(); }

        private:
            std::string	m_description;

            RenderPassConcept*	m_pass;
        };

    }
}

#endif // !RenderPass_hpp
