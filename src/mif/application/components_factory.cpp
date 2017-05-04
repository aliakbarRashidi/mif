//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     04.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// MIF
#include "mif/application/id/service.h"
#include "mif/application/iconfig.h"
#include "mif/service/creator.h"
#include "mif/service/ifactory.h"

namespace Mif
{
    namespace Application
    {
        namespace Detail
        {
            namespace
            {

                class ComponentsFactory
                    : public Service::Inherit<Service::IFactory>
                {
                public:
                    ComponentsFactory(IConfigPtr componentsInfo)
                    {
                        if (!componentsInfo)
                        {
                            throw std::invalid_argument{"[Mif::Application::Detail::ComponentsFactory] "
                                    "Empty input components info."};
                        }

                        if (componentsInfo->Exists("components"))
                        {
                            auto components = componentsInfo->GetCollection("components");
                            components->Reset();
                            for (auto next = !components->IsEmpty() ; next ; next = components->Next())
                            {
                                components->Get();
                            }
                        }
                    }

                private:
                    // IFactory
                    virtual Service::IServicePtr Create(Service::ServiceId id) override final
                    {
                        (void)id;
                        throw std::runtime_error{"Not implemented"};
                    }

                    virtual Service::IServicePtr Create(std::string const &id) override final
                    {
                        (void)id;
                        throw std::runtime_error{"Not implemented"};
                    }
                };

            }   // namespace
        }   // namespace Detail
    }   // namespace Application
}   // namespace Mif

MIF_SERVICE_CREATOR
(
    Mif::Application::Id::Service::ComponentsFactory,
    Mif::Application::Detail::ComponentsFactory,
    Mif::Application::IConfigPtr
)
